#include "DlibUtilities.h"
#include "dlib/opencv.h"

dlib::array2d<dlib::bgr_pixel> DlibUtilities::cvMatToDLibBgr(const cv::Mat & cvMatBgr)
{
	// BGR cv::Mat -> dlib::array2d
	dlib::array2d<dlib::bgr_pixel> dlibPixelsRGBA1;
	dlib::assign_image(dlibPixelsRGBA1, dlib::cv_image<dlib::bgr_pixel>(cvMatBgr));
	return dlibPixelsRGBA1;
}

dlib::array2d<dlib::rgb_pixel> DlibUtilities::cvMatToDLibRgb(const cv::Mat & cvMatRgb)
{
	// BGR cv::Mat -> dlib::array2d
	dlib::array2d<dlib::rgb_pixel> dlibPixelsRGBA1;
	dlib::assign_image(dlibPixelsRGBA1, dlib::cv_image<dlib::rgb_pixel>(cvMatRgb));
	return dlibPixelsRGBA1;
}
