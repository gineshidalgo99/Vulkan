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

	// Reset Landmarks
	for (auto i = 0; i < m_landmarksPrevious.size(); i++)
		for (auto j = 0; j < m_landmarksPrevious[0].size(); j++)
			m_landmarksPrevious[i][j] = { 0 };
}

std::tuple<bool, Landmarks, dlib::rectangle> DlibFaceTracker::getFaceLandmarks(const dlib::array2d<dlib::bgr_pixel> & image, const int offsetX, const int offsetY)
{
	bool landmarksFound;
	Landmarks landmarks;
	dlib::rectangle higherFaceRect;

	// Face detection
	auto pyrDownRatio = 1;
	dlib::array2d<dlib::bgr_pixel> imageDown;
	const auto minMaxSize = (512 * 512);
	if (image.size() > minMaxSize)
	{
		m_pyrDown2(image, imageDown);
		pyrDownRatio *= 2;
	}
	while (imageDown.size() > minMaxSize)
	{
		m_pyrDown2(imageDown, imageDown);
		pyrDownRatio *= 2;
	}
	const auto faceRects(m_faceDetector((pyrDownRatio > 1 ? imageDown : image)));

	if (faceRects.size() > 0)
	{
		// Keep only 1 face rect / image (the higher one)
		higherFaceRect = { keepHigherRectangle(faceRects) };

		// Face detection re-sizing
		resizeRectangle(pyrDownRatio, higherFaceRect);
	}


	landmarksFound = { higherFaceRect != dlib::rectangle{} };
	if (landmarksFound)
	{
		// Face tracking
		const auto dlibLandmarks = m_sp(image, higherFaceRect);
		landmarksFound = { dlibLandmarks.num_parts() > 0 };

		if (landmarksFound)
		{
			for (auto i = 0ul; i < dlibLandmarks.num_parts(); i++)
			{
				landmarks[0][i] = { (float)dlibLandmarks.part(i)(0, 0) + offsetX };
				landmarks[1][i] = { (float)dlibLandmarks.part(i)(0, 1) + offsetY };

				// Smoothing
				for (auto j = 0; j < landmarks.size(); j++)
				{
					const auto absoluteValue = std::abs(m_landmarksPrevious[j][i] - landmarks[j][i]);
					const auto sign = std::signbit(m_landmarksPrevious[j][i] - landmarks[j][i]);

					if (absoluteValue > 0)
					{
						if (absoluteValue < 2)
							landmarks[j][i] = { m_landmarksPrevious[j][i] + sign };
						else if (absoluteValue < 4)
							landmarks[j][i] = { m_landmarksPrevious[j][i] + 2 * sign };
						else if (absoluteValue < 6)
							landmarks[j][i] = { m_landmarksPrevious[j][i] + 3 * sign };
						else if (absoluteValue < 10)
							landmarks[j][i] = { m_landmarksPrevious[j][i] + 5 * sign };
						//else if (absoluteValue < 15)
						//	landmarks[j][i] = { m_landmarksPrevious[j][i] + 10 * sign };
						//else if (absoluteValue < 20)
						//	landmarks[j][i] = { m_landmarksPrevious[j][i] + 15 * sign };
					}
				}
			}
		}
		else
			higherFaceRect = { dlib::rectangle{} };
	}

	// Add offset
	if (higherFaceRect != dlib::rectangle{})
	{
		higherFaceRect.set_left(higherFaceRect.left() + offsetX);
		higherFaceRect.set_right(higherFaceRect.right() + offsetX);
		higherFaceRect.set_top(higherFaceRect.top() + offsetY);
		higherFaceRect.set_bottom(higherFaceRect.bottom() + offsetY);
	}

	// Update landmarks
	if (landmarksFound)
		m_landmarksPrevious = { landmarks };

	// Save previousFaceRect for next iteration
	return std::make_tuple(landmarksFound, landmarks, higherFaceRect);
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