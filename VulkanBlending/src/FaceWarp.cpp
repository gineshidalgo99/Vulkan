#include <chrono>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp> 
#include "FaceWarp.h"

std::tuple<cv::Mat, cv::Rect> FaceWarp::face_warp(const cv::Mat & image1, const cv::Mat & image2, const Landmarks & landmarks1, const Landmarks & landmarks2)
{
	const auto minX_dst = *std::min_element(landmarks2[0].begin() + 17, landmarks2[0].end());
	const auto minY_dst = *std::min_element(landmarks2[1].begin() + 17, landmarks2[1].end());
	const auto maxX_dst = *std::max_element(landmarks2[0].begin() + 17, landmarks2[0].end());
	const auto maxY_dst = *std::max_element(landmarks2[1].begin() + 17, landmarks2[1].end());

	std::vector<cv::Point2f> pts_src(landmarks1[0].size());
	std::vector<cv::Point2f> pts_dst(landmarks1[0].size());
    for (auto i = 0; i < pts_src.size(); i++)
    {
		pts_src[i] = { landmarks1[0][i], landmarks1[1][i] };
		pts_dst[i] = { landmarks2[0][i], landmarks2[1][i] };
    }

	const auto h = cv::findHomography(pts_src, pts_dst);
	const auto finalWH = (int)std::round(std::max((maxX_dst - minX_dst), (maxY_dst - minY_dst)));
	const auto delta = std::abs((maxX_dst - minX_dst) - (maxY_dst - minY_dst));

	// Make faceRect squared
	cv::Rect dst_rect;
	if ((maxX_dst - minX_dst) > (maxY_dst - minY_dst))
		dst_rect = { cv::Rect{ (int)std::round(minX_dst), (int)std::round(minY_dst - delta / 2.f), finalWH, finalWH } };
	else
		dst_rect = { cv::Rect{ (int)std::round(minX_dst - delta / 2.f), (int)std::round(minY_dst), finalWH, finalWH } };

	// Perform warping
	cv::Mat image1Warped;
    cv::warpPerspective(image1, image1Warped,h, image2.size());

	// Illumination normalization (only applied over the faceRect and surroundings)
	const auto increase = 0.175f;
	const auto x = std::max(0, (int)std::round(dst_rect.x - dst_rect.width * increase));
	const auto y = std::max(0, (int)std::round(dst_rect.y - dst_rect.height * increase));
	const auto wh = (int)std::round(dst_rect.width * (1 + 2 * increase));
	const auto faceWarped = cv::Mat{ image1Warped, cv::Rect{ x,
															 y,
															 std::min(image1Warped.cols - x, wh),
															 std::min(image1Warped.rows - y, wh) } };
	const auto meanImage1Warped = cv::mean(cv::Mat{ image1Warped, dst_rect });
	const auto meanImage2 = cv::mean(cv::Mat{ image2, dst_rect });
	faceWarped += (meanImage2 - meanImage1Warped);

    return std::make_tuple(image1Warped,dst_rect);
}

//int main(int argc, char** argv)
//{
//    FaceWarp::Pts2warp landmarks;
//    ifstream myfile1("v1.txt");
//    if(myfile1.is_open())
//    {
//        for (int i = 0; i < 68; ++i)
//        {
//            myfile1 >> landmarks1[0][i];
//        }
//        for (int i = 68; i < 136; ++i)
//        {
//            myfile1 >> landmarks2[0][i-68];
//        }
//
//    }
//    ifstream myfile2("v2.txt");
//    if(myfile2.is_open())
//    {
//        for (int i = 0; i < 68; ++i)
//        {
//            myfile2 >> landmarks1[1][i];
//        }
//        for (int i = 68; i < 136; ++i)
//        {
//            myfile2 >> landmarks2[1][i-68];
//        }
//
//    }
//    Rect myRect;
//    auto mytuple = FaceWarp::face_warp(argv[1],argv[2],landmarks);
//    imshow("warped image",get<0>(mytuple));
//    waitKey(0);
//    cout << get<1>(mytuple)<< endl;
//
//    return 0;
//}
