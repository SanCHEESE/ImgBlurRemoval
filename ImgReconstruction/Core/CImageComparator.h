//
//  ImageComparator.h
//  ImgReconstruction
//
//  Created by Alexander Bochkarev on 27.09.15.
//  Copyright © 2015 Alexander Bochkarev. All rights reserved.
//

#include "IImageComparator.h"

class CImageComparator : public IImageComparator
{
public:
	CImageComparator(TImageCompareMetric compMetric = TImageCompareMetricL1, TBrightnessEqualization brightnessEqualization = TBrightnessEqualizationMean, TCompSum compSum = TCompSumStd,
                     double borderWeight = 1) :
        _compMetric(compMetric), _brightnessEqualization(brightnessEqualization), _compSum(compSum), _borderWeight(borderWeight) {};
    
    virtual double operator()(const CImagePatch& patch1, const CImagePatch& patch2) const;
private:
	double CompareL1(const CImagePatch& patch1, const CImagePatch& patch2) const;
	double CompareL2(const CImagePatch& patch1, const CImagePatch& patch2) const;
    
    void EqualizeBrightness(CImage &img1, CImage &img2) const;
    void EqualizeBrightnessMean(CImage &img1, CImage &img2) const;
    void EqualizeBrightnessDynRange(CImage &img1, CImage &img2) const;
    
    double Sum(const CImage& img) const;
	
	TImageCompareMetric _compMetric;
    TBrightnessEqualization _brightnessEqualization;
    TCompSum _compSum;
    
    double _borderWeight;
};