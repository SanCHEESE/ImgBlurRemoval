#include "CBlurMetricsTester.h"

#include <CDynamicRangeBlurMeasurer.hpp>
#include <CFDBlurMeasurer.hpp>
#include <CFFTBlurMeasurer.hpp>
#include <CStdDeviationBlurMeasurer.hpp>

#include <CPatchFetcher.hpp>
#include <CImageExtender.hpp>

#include <CPatchFilter.hpp>
#include <CNiBlackBinarizer.hpp>

#include <utils.h>


void CBlurMetricsTester::Test()
{
	const int bigPatchSize = 300;
	const int bigPatchOffset = 300;
	const int minContrastValue = 40;
	const int minPatchSize = 8;
	const int maxPatchSize = 25;

	/* fetch big patches */
	CImageExtender extender({bigPatchSize, bigPatchSize});
	CImage extentImage = extender.Extent(_image);
	CPatchFetcher patchFetcher({bigPatchSize, bigPatchSize}, {bigPatchOffset , bigPatchOffset}, 0);
	auto bigPatches = patchFetcher.FetchPatches(extentImage);

	/* smooth images */
	int tag = 0;
	int kernelSize = 3;
	int maxKernelSize = 27;
	for (CImage& bigImg : bigPatches) {
		cv::GaussianBlur(bigImg, bigImg, cv::Size(kernelSize, kernelSize), 0, 0);

		bigImg.Save();

		bigImg.tag = tag;
		tag++;

		kernelSize = kernelSize > maxKernelSize ? 3 : (kernelSize + 2);
	}

	/* iterate over patch sizes */
	for (int patchSize = minPatchSize; patchSize < maxPatchSize; patchSize++) {
		std::vector<CImage> patchesToTest;

		IBinarizer* binarizer = new CNiBlackBinarizer({patchSize, patchSize}, -0.2);
		IPatchFilter* filter = new CPatchFilter(binarizer, minContrastValue, {patchSize / 4, patchSize / 4});
		CPatchFetcher* patchFetcher = new CPatchFetcher({patchSize, patchSize}, {patchSize, patchSize}, filter);

		for (CImage &bigImage : bigPatches) {
			CImageExtender extender({patchSize, patchSize});
			CImage extended = extender.Extent(bigImage);

			auto patches = patchFetcher->FetchPatches(extended);

			if (patches.size() > 1) {
				patchesToTest.push_back(patches[0]);
				patchesToTest[patchesToTest.size() - 1].tag = bigImage.tag;
			}
		}

		auto buildTagSequence = [](const std::vector<CImage>& patches) -> std::string {
			std::string s;
			std::stringstream out;
			for (const CImage& patch : patches) {
				out << patch.tag;
			}
			return out.str();
		};

		std::string originalTagSequence = buildTagSequence(patchesToTest);

		for (TBlurMeasureMethod method = TBlurMeasureMethodStandartDeviation; method <= TBlurMeasureMethodFD; method = (TBlurMeasureMethod)((int)method + 1)) {
			IBlurMeasurer* blurMeasurer = BlurMeasurerForMethod(method);

			auto patchesCopy(patchesToTest);

			std::sort(patchesCopy.begin(), patchesCopy.end(), [&](const CImage& img1, const CImage& img2) -> bool {
				return blurMeasurer->Measure(img1) > blurMeasurer->Measure(img2);
			});

			std::string tagSequence = buildTagSequence(patchesCopy);

			int distance = utils::LevensteinDistance(originalTagSequence, tagSequence);

			std::cout << "Patch size: " << patchSize << "x" << patchSize << std::endl <<
				"Method " << MethodNameForMethod(method) << " "
				"Errors: " << (float)distance / (float)patchesCopy.size() << "%" << std::endl;

			delete blurMeasurer;
		}

		delete patchFetcher;
		delete filter;
		delete patchFetcher;
	}
}

std::string CBlurMetricsTester::MethodNameForMethod(TBlurMeasureMethod method) const
{
	switch (method) {
		case TBlurMeasureMethodFFT:
			return "TBlurMeasureMethodFFT";
		case TBlurMeasureMethodDynamicRange:
			return "TBlurMeasureMethodDynamicRange";
		case TBlurMeasureMethodFD:
			return "TBlurMeasureMethodFD";
		case TBlurMeasureMethodStandartDeviation:
			return "TBlurMeasureMethodStandartDeviation";
		default:
			break;
	}
	return 0;
}

IBlurMeasurer* CBlurMetricsTester::BlurMeasurerForMethod(TBlurMeasureMethod method) const
{
	switch (method) {
		case TBlurMeasureMethodFFT:
			return new CFFTBlurMeasurer(0.3);
		case TBlurMeasureMethodDynamicRange:
			return new CDynamicRangeBlurMeasurer();
		case TBlurMeasureMethodFD:
			return new CFDBlurMeasurer();
		case TBlurMeasureMethodStandartDeviation:
			return new CStdDeviationBlurMeasurer();
		default:
			break;
	}
	return 0;
}