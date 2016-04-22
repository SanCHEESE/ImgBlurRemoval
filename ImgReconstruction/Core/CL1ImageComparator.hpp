//
//  CL1ImageComparator.hpp
//  ImgReconstruction
//
//  Created by Alexander Bochkarev on 14.03.16.
//  Copyright © 2016 Alexander Bochkarev. All rights reserved.
//

#pragma once

#include "CImageComparator.hpp"

class CL1ImageComparator : public CImageComparator
{
public:
	using CImageComparator::CImageComparator;

	virtual bool Equal(const CImagePatch& patch1, const CImagePatch& patch2) const
	{
		CImage normPatch1;
		patch1.GrayImage().copyTo(normPatch1);

		CImage normPatch2;
		patch2.GrayImage().copyTo(normPatch2);

		_equalizer->EqualizeBrightness(normPatch1, normPatch2);

		CImage result;
		cv::absdiff(normPatch1, normPatch2, result);

		float sum = _summator->Sum(result);

		return sum < _eps;
	}
};
