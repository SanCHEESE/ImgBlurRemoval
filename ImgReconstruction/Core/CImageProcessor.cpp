//  CImageProcessor.cpp
//  ImgReconstruction
//
//  Created by Alexander Bochkarev on 27.09.15.
//  Copyright © 2015 Alexander Bochkarev. All rights reserved.
//

#include "CImageProcessor.h"
#include "CTimeLogger.h"
#include "CAccImage.h"

void CImageProcessor::ProcessImage(const CImage& img, const std::string& outImagePath)
{
	_outImagePath = outImagePath;

	RestoreImageIteratively(_iterCount, img);
}

void CImageProcessor::GenerateHelperImages(const CImage& img)
{
	_origImageSize = img.GetSize();

	CImage extentImage = _subprocHolder->ImageExtender()->Extent(img);
	CImage blurredImage;
	cv::bilateralFilter(extentImage, blurredImage, 2, 1, 1);
	_mainImage = CImagePatch(extentImage, _subprocHolder->PatchBinarizer()->Binarize(blurredImage));
}

CImage CImageProcessor::RestoreImageIteratively(int iterCount, const CImage& img)
{
	CImage image = img;
	for (int iter = 0; iter < iterCount; iter++) {
		GenerateHelperImages(image);
		image = RestoreImage();
		image = image({0, 0, _origImageSize.width, _origImageSize.height});
	}

	image.Save(_outImagePath, 100, "");

	return image;
}

CImage CImageProcessor::RestoreImage()
{
	// get all image patches
	std::vector<CImagePatch> patches = _subprocHolder->PatchFetcher()->FetchPatches(_mainImage);

	//std::cout << "Total patches: " << patches.size() << std::endl;

	// calculating
	for (CImagePatch& patch : patches) {
		patch.BlurValue(_subprocHolder->BlurMeasurer());
	}

	// classification by PHash/AvgHash
	std::map<uint64, std::vector<CImagePatch>> classes = _subprocHolder->PatchClassifier()->Classify(patches);

	CAccImage accImage(_mainImage.GrayImage());

	for (auto &it : classes) {
		std::vector<CImagePatch> aClass = it.second;
		if (aClass.size() < 2) {
			// do not process classes with size of 1 object
			accImage.SetImageRegion(aClass[0].GrayImage());

			continue;
		} else {
			// ranking by sharpness inside a class
			auto clusters = Clusterize(aClass);

			for (auto& cluster : clusters) {
				auto clusterPatches = cluster.second;

				// sorting by blur increase
				std::sort(clusterPatches.begin(), clusterPatches.end(), MoreBlur());

#ifdef _DEBUG
				for (auto patch : clusterPatches) {
					std::cout << patch.GetBlurValue() << std::endl;
				}
#endif

				// copying to summing image
				CImagePatch bestPatch = clusterPatches[0];
				for (auto& patch : clusterPatches) {
					double blurThresh = std::abs((bestPatch.GetBlurValue() - patch.GetBlurValue()));
					// copy if threshhold in relatively big
					if (patch.GetFrame() != bestPatch.GetFrame() && _config.blurThresh < blurThresh) {
						accImage.SetImageRegion(bestPatch.GrayImage(), patch.GetFrame());
					}
				}
			}
		}
	}

	return accImage.GetResultImage(_config.accImageSumMethod);
}
