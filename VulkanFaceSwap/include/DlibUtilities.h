#ifndef DLIB_UTILITIES_H
#define DLIB_UTILITIES_H

#include <opencv2/opencv.hpp>
#include "dlib/image_processing.h"

class DlibUtilities
{
	public:
		static dlib::array2d<dlib::bgr_pixel> cvMatToDLibBgr(const cv::Mat & cvMatBgr);
		static dlib::array2d<dlib::rgb_pixel> cvMatToDLibRgb(const cv::Mat & cvMatRgb);
};

#endif // DLIB_UTILITIES_H
