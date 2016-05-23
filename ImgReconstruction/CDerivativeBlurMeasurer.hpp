#pragma once

#include "IBlurMeasurer.h"

#include "utils.h"

#include <numeric>

class CDerivativeBlurMeasurer : public IBlurMeasurer
{
public:
	CDerivativeBlurMeasurer(int extentPixels, float cutPercentage, TBlurMeasurerDerivativeCalcMethod method)
	{
		_extentPixels = extentPixels;
		_cutPercentage = cutPercentage;
		_method = method;
	}

	virtual float Measure(const CImage& img, const CImage* const parentImg = 0) const
	{
		cv::Rect2f imgFrame = img.GetFrame();

		cv::Rect extentFrame;
		if (imgFrame.x < _extentPixels) {
			// near left border
			extentFrame.x = 0;
		} else if (parentImg->cols <= imgFrame.x + imgFrame.width + _extentPixels) {
			// near right border
			extentFrame.x = imgFrame.x - _extentPixels - (_extentPixels - (parentImg->cols - imgFrame.x - imgFrame.width));
		} else {
			// in the middle
			extentFrame.x = imgFrame.x - _extentPixels;
		}
		extentFrame.width = img.cols + 2 * _extentPixels;

		if (imgFrame.y < _extentPixels) {
			// near top border
			extentFrame.y = 0;
		} else if (parentImg->rows <= imgFrame.y + imgFrame.height + _extentPixels) {
			// near right border
			extentFrame.y = imgFrame.y - _extentPixels - (_extentPixels - (parentImg->rows - imgFrame.y - imgFrame.height));
		} else {
			// in the middle
			extentFrame.y = imgFrame.y - _extentPixels;
		}
		extentFrame.height = img.rows + 2 * _extentPixels;

		CImage extentImage = (*parentImg)(extentFrame);

		std::vector<int> dx(255, 0);
		std::vector<int> dy(255, 0);
		std::vector<int> dxy(255, 0);
		std::vector<int> dyx(255, 0);

		for (int i = 1; i < extentImage.rows - 1; i++) {
			for (int j = 0; j < extentImage.cols - 1; j++) {
				int delta = extentImage.at<uchar>(i + 1, j) - extentImage.at<uchar>(i, j); // dy
				dy[std::abs(delta)] += 1;

				delta = extentImage.at<uchar>(i, j + 1) - extentImage.at<uchar>(i, j); // dx
				dx[std::abs(delta)] += 1;

				delta = extentImage.at<uchar>(i + 1, j + 1) - extentImage.at<uchar>(i, j); // dxy
				dxy[std::abs(delta)] += 1;

				delta = extentImage.at<uchar>(i - 1, j + 1) - extentImage.at<uchar>(i, j); // dyx
				dyx[std::abs(delta)] += 1;
			}
		}

		for (int j = 0; j < extentImage.cols - 1; j++) {
			int delta = extentImage.at<uchar>(0, j + 1) - extentImage.at<uchar>(0, j); // dx
			dx[std::abs(delta)] += 1;
		}

		for (int i = 0; i < extentImage.rows - 1; i++) {
			int delta = extentImage.at<uchar>(i + 1, extentImage.cols - 1) - extentImage.at<uchar>(i, extentImage.cols - 1); // dy
			dy[std::abs(delta)] += 1;
		}

		for (int j = 0; j < extentImage.cols - 1; j++) {
			int delta = extentImage.at<uchar>(extentImage.rows - 2, j + 1) - extentImage.at<uchar>(extentImage.rows - 1, j); // dyx
			dyx[std::abs(delta)] += 1;
		}

		int i = 0;
		int total = std::accumulate(dx.begin(), dx.end(), 0);
		int histDx = 0;
		int dxBlur = 0;
		for (auto it = dx.begin(); it != dx.end(); it++, i++) {
			histDx += *it;
			if ((float)histDx/total >= 1 - (float)_cutPercentage/100.f) {
				dxBlur = i;
				break;
			}
		}

		total = std::accumulate(dy.begin(), dy.end(), 0);
		int histDy = 0;
		int dyBlur = 0;
		i = 0;
		for (auto it = dy.begin(); it != dy.end(); it++, i++) {
			histDy += *it;
			if ((float)histDy / total >= 1 - (float)_cutPercentage / 100.f) {
				dyBlur = i;
				break;
			}
		}

		total = std::accumulate(dxy.begin(), dxy.end(), 0);
		int histDxy = 0;
		int dxyBlur = 0;
		i = 0;
		for (auto it = dxy.begin(); it != dxy.end(); it++, i++) {
			histDxy += *it;
			if ((float)histDxy / total >= 1 - (float)_cutPercentage / 100.f) {
				dxyBlur = i;
				break;
			}
		}

		total = std::accumulate(dyx.begin(), dyx.end(), 0);
		int histDyx = 0;
		int dyxBlur = 0;
		i = 0;
		for (auto it = dyx.begin(); it != dyx.end(); it++, i++) {
			histDyx += *it;
			if ((float)histDyx / total >= 1 - (float)_cutPercentage / 100.f) {
				dyxBlur = i;
				break;
			}
		}

		std::vector<int> derivatives;
		derivatives.push_back(dxBlur);
		derivatives.push_back(dyBlur);
		derivatives.push_back(dxyBlur);
		derivatives.push_back(dyxBlur);

		return CalculateBlurValue(derivatives);

	}
private:
	float CalculateBlurValue(const std::vector<int>& derivatives) const
	{
		float result = 0;
		switch (_method) {
			case TBlurMeasurerDerivativeCalcMethodAvg:
				result = std::accumulate(derivatives.begin(), derivatives.end(), 0.0);
				result /= derivatives.size();
				break;
			case TBlurMeasurerDerivativeCalcMethodMax:
				result = *std::max_element(derivatives.begin(), derivatives.end());
				break;
			case TBlurMeasurerDerivativeCalcMethodMin:
				result = *std::min_element(derivatives.begin(), derivatives.end());
				break;
			case TBlurMeasurerDerivativeCalcMethodSum:
				result = std::accumulate(derivatives.begin(), derivatives.end(), 0.0);
				break;
			default:
				break;
		}

		return result;
	}

	int _extentPixels;
	float _cutPercentage;
	TBlurMeasurerDerivativeCalcMethod _method;
};