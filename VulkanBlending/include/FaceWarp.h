#ifndef FACE_WARP_HPP_
#define FACE_WARP_HPP_

#include <fstream>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "Landmarks.h"

class FaceWarp
{
public:    
    static std::tuple<cv::Mat, cv::Rect> face_warp(const cv::Mat & image1, const cv::Mat & image2, const Landmarks & landmarks1, const Landmarks & landmarks2);
};

#endif
