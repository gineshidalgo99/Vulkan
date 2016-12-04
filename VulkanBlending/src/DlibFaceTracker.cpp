#include <fstream>
#include <iostream>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <opencv2/opencv.hpp>
#include "DlibFaceTracker.h"

DlibFaceTracker::DlibFaceTracker(const char* trainFaceModel) :
	m_faceDetector{ dlib::get_frontal_face_detector() }
{
	dlib::deserialize(trainFaceModel) >> m_sp;
}

std::pair<bool, Landmarks> DlibFaceTracker::getFaceLandmarks(const dlib::array2d<dlib::bgr_pixel> & image, dlib::rectangle & previousFaceRect)
{
	auto landmarksFound = true;
	Landmarks landmarks;

	// Face detection
	dlib::rectangle higherFaceRect;
	if (previousFaceRect == dlib::rectangle{})
	{
std::cout << image.size() << std::endl;
		auto pyrDownRatio = 1;
		dlib::array2d<dlib::bgr_pixel> imageDown;
		if (image.size() > (512 * 512))
		{
			m_pyrDown2(image, imageDown);
			pyrDownRatio *= 2;
		}
		while (imageDown.size() > (512 * 512))
		{
			m_pyrDown2(imageDown, imageDown);
			pyrDownRatio *= 2;
		}
std::cout << imageDown.size() << std::endl;
		const auto faceRects(m_faceDetector((pyrDownRatio > 1 ? imageDown : image)));

		if (faceRects.size() > 0)
		{
			// Keep only 1 face rect / image (the higher one)
			higherFaceRect = { keepHigherRectangle(faceRects) };

			// Face detection re-sizing
			resizeRectangle(pyrDownRatio, higherFaceRect);
		}
	}
	else
		higherFaceRect = { previousFaceRect };


	landmarksFound = { higherFaceRect != dlib::rectangle{} };
	if (landmarksFound)
	{
		// Face tracking
		const auto dlibLandmarks = m_sp(image, higherFaceRect);
		landmarksFound = (dlibLandmarks.num_parts() > 0);

		if (landmarksFound)
		{
			for (unsigned long i = 0; i < dlibLandmarks.num_parts(); ++i)
			{
				landmarks[0][i] = dlibLandmarks.part(i)(0, 0);
				landmarks[1][i] = dlibLandmarks.part(i)(0, 1);
			}

//for (auto i = 0; i < landmarks.size(); i++)
//{
//	for (auto j = 0; j < landmarks[i].size(); j++)
//		std::cout << landmarks[i][j] << " ";
//	std::cout << std::endl;
//}

			// Save previousFaceRect for next iteration
			//if (previousFaceRect.area() <= 1)
			{
				// Get max and min
				//std::cout << *std::min_element(landmarks[0].begin(), landmarks[0].end()) << std::endl;
				//std::cout << *std::max_element(landmarks[0].begin(), landmarks[0].end()) << std::endl;
				const auto constant = 0;
				previousFaceRect.set_left(-constant + *std::min_element(landmarks[0].begin(), landmarks[0].end()));
				previousFaceRect.set_right(constant + *std::max_element(landmarks[0].begin(), landmarks[0].end()));
				previousFaceRect.set_top(-constant + *std::min_element(landmarks[1].begin(), landmarks[1].end()));
				previousFaceRect.set_bottom(constant + *std::max_element(landmarks[1].begin(), landmarks[1].end()));
				previousFaceRect = higherFaceRect;
			}
		}
		//else
			previousFaceRect = { dlib::rectangle{} };
//std::cout << previousFaceRect << std::endl;
//previousFaceRect = { dlib::rectangle{} };
//std::cout << previousFaceRect << std::endl;
//std::cout << __LINE__ << std::endl;
	}

	// Save previousFaceRect for next iteration
	if (!landmarksFound)
		previousFaceRect = { dlib::rectangle{} };

	return std::make_pair(landmarksFound, landmarks);
}

void DlibFaceTracker::resizeRectangle(const int factor, dlib::rectangle & rectangle)
{
	rectangle.set_left(factor * rectangle.left());
	rectangle.set_top(factor * rectangle.top());
	rectangle.set_right(factor * rectangle.right());
	rectangle.set_bottom(factor * rectangle.bottom());
}

dlib::rectangle DlibFaceTracker::keepHigherRectangle(const std::vector<dlib::rectangle> & rectangles)
{
	auto maxArea = rectangles[0].area();
	auto maxAreaIndex = 0;
	for (auto i = 1; i < rectangles.size(); ++i)
	{
		if (rectangles[i].area() > maxArea)
		{
			maxArea = rectangles[i].area();
			maxAreaIndex = i;
		}
	}

	return rectangles[maxAreaIndex];
}

//int main(int argc, char** argv)
//{
//
//    DlibFaceTracker::Pts2 landmarks = DlibFaceTracker::getFaceLandmarks(argv[1],argv[2],argv[3]);
//    for (int i = 0; i < 68; ++i)
//    {
//        cout << landmarks.v1[0][i] << " " << landmarks.v1[1][i] << endl;
//    }
//
//    for (int i = 0; i < 68; ++i)
//    {
//        cout << landmarks.v2[0][i] << " " << landmarks.v2[1][i] << endl;
//    }
//return 0;
//}