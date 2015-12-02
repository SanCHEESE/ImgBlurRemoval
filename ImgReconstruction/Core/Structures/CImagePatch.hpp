//
//  CImagePatch.hpp
//  ImgReconstruction
//
//  Created by Alexander Bochkarev on 24.10.15.
//  Copyright © 2015 Alexander Bochkarev. All rights reserved.
//

#pragma once

#include "CBlurMeasurer.hpp"
#include "utils.hpp"

class CImagePatch
{
public:
    CImagePatch() {Initialize();}
    CImagePatch(const CImage& grayImage, const CImage& binImage, const CImage& sdImage) {Initialize();SetGrayImage(grayImage);SetBinImage(binImage);SetSdImage(sdImage);};
    
    double BlurValue(TBlurMeasureMethod method);
    double StandartDeviation();
    int64 PHash();
    int64 AvgHash();
    
    // accessors
    CImage GrayImage() const {return _grayImage;}
    CImage BinImage() const {return _binImage;}
    CImage SdImage() const {return _sdImage;}
    
    // setters
    void SetGrayImage(const CImage& image) {
        image.copyTo(_grayImage);
        _frame = _grayImage.GetFrame();
    };
    void SetBinImage(const CImage& image) {
        image.copyTo(_binImage);
        _frame = _binImage.GetFrame();
    };
    void SetSdImage(const CImage& image) {
        image.copyTo(_sdImage);
        _frame = _sdImage.GetFrame();
    };

    double GetBlurValue() const {return _blurValue;};
    double GetStandartDeviation() const {return _standartDeviation;};
    int64 GetPHash() const {return _pHash;};
    int64 GetAvgHash() const {return _avgHash;};
    cv::Rect GetFrame() const {return _frame;};
    
    friend std::ostream& operator<<(std::ostream& os, const CImagePatch& patch);
    
    double distanceToTarget;
private:
    void Initialize();
    double CalculateBlurValue(TBlurMeasureMethod method) const;
    double CalculateStandartDeviation() const;
    
    cv::Rect _frame;
    CImage _grayImage;
    CImage _binImage;
    CImage _sdImage;
    
    int _imgClass;
    double _blurValue;
    double _standartDeviation;
    
    int64 _avgHash;
    bool _avgHashComputed;
    int64 _pHash;
    bool _pHashComputed;
};

struct LessBlur
{
    inline bool operator() (const CImagePatch& patch1, const CImagePatch& patch2)
    {
        return patch1.GetBlurValue() > patch2.GetBlurValue();
    }
};

struct LessSimilarity
{
    inline bool operator() (const CImagePatch& patch1, const CImagePatch& patch2)
    {
        return patch1.distanceToTarget < patch2.distanceToTarget;
    }
};
