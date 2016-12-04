#ifndef OPEN_CV_PROCESSING_H
#define OPEN_CV_PROCESSING_H

#include <opencv2/opencv.hpp>
#include "DlibFaceTracker.h"
#include "DlibUtilities.h"
#include "FaceWarp.h"
#include "FramesProducer.h"
#include "Utilities.h"

const auto modeVideo = true;

class FramesProcessing
{
public:
	FramesProcessing() :
		m_dlibFaceTracker{ "../model/shape_predictor_68_face_landmarks.dat" },
		m_framesProducer{(modeVideo ? "0" : "9"), FramesProducer::FrameMirrorMode::NoMirrored, FramesProducer::FrameRotation::Degrees0}
	{
		// Loading face to swap
		// Image 1 (if no video)
		if (!modeVideo)
		{
			m_image1rgba = { readImageFromFile("textures/texture1.jpg") };
			cv::cvtColor(m_image1rgba, m_image1rgba, CV_BGR2RGBA);
		}

		// Image 2
		m_image2rgba = { readImageFromFile("textures/texture1.jpg") };
		// Resize if required
		if (!modeVideo && m_image1rgba.size() != m_image2rgba.size())
			cv::resize(m_image2rgba, m_image2rgba, m_image1rgba.size(), 0.0, 0.0, CV_INTER_AREA);
		// RGB cv::Mat -> dlib::array2d
		m_image2bgrDlib = { DlibUtilities::cvMatToDLibBgr(m_image2rgba) };
		// RGB to RGBA
		cv::cvtColor(m_image2rgba, m_image2rgba, CV_BGR2RGBA);

		// Landmarks image 2
		bool landmarks2Found;
		std::tie(landmarks2Found, m_landmarksImage2) = m_dlibFaceTracker.getFaceLandmarks(m_image2bgrDlib);

		// To check mask
		//m_image1rgba.setTo(0);
		//m_image2rgba.setTo(255);
	}

	cv::Mat getNextImage()
	{
const auto clockIteration = std::chrono::high_resolution_clock::now();
		auto frameState = FramesProducer::FrameState::OK;
		cv::Mat image1rgba;
		dlib::array2d<dlib::bgr_pixel> dlibImage1Bgr;
		if (!modeVideo)
		{
			// cv::Mat image
			image1rgba = { m_image1rgba };
			// Dlib image
			cv::Mat temp;
			cv::cvtColor(m_image1rgba, temp, CV_RGBA2RGB);
			dlibImage1Bgr = { DlibUtilities::cvMatToDLibBgr(temp) };
		}
		// Video
		if (modeVideo)
		{
			std::tie(frameState, image1rgba) = m_framesProducer.getValidFrame();
			if (frameState == FramesProducer::FrameState::OK && !image1rgba.empty())
			{
				// BGR cv::Mat -> dlib::array2d
				dlibImage1Bgr = { DlibUtilities::cvMatToDLibBgr(image1rgba) };
				// image1rgba BGR to RGBA
				cv::cvtColor(image1rgba, image1rgba, CV_BGR2RGBA);
			}

		}
		// Fix image
		else
			image1rgba = { m_image1rgba.clone() };
const auto ms_1 = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);

		if (frameState == FramesProducer::FrameState::OK && !image1rgba.empty() && !m_image2rgba.empty())
		{
			// Face landmarks tracking
			bool landmarks1Found;
			Landmarks landmarks1;
			std::tie(landmarks1Found, landmarks1) = m_dlibFaceTracker.getFaceLandmarks(dlibImage1Bgr, m_previousRectangleImage1);
const auto ms_2 = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6) ;
			// Homography to align faces
			cv::Mat image2rgbaWarped;
			cv::Rect faceRect;
			if (landmarks1Found)
				std::tie(image2rgbaWarped, faceRect) = FaceWarp::face_warp(m_image2rgba, image1rgba, m_landmarksImage2, landmarks1);
			else
			{
				image2rgbaWarped = { m_image2rgba };
				faceRect = { 1,1,2,2 };
			}
const auto ms_3 = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);
			//image2rgbaWarped = m_image2rgba;

			// size(Image2warped) = size(image1)
			if (image1rgba.size() != image2rgbaWarped.size())
				padOrCropToFixedSize(image2rgbaWarped, image1rgba.size());
				//cv::resize(image2rgbaWarped, image2rgbaWarped, image1rgba.size(), 0.0, 0.0, CV_INTER_AREA);	// Resize not valid -> it would change face location
const auto ms_4 = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);

			setMask(image2rgbaWarped, faceRect);
const auto ms_5 = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);
			if (m_pixelsRgba.empty())
				m_pixelsRgba = { cv::Mat{ image1rgba.rows + image2rgbaWarped.rows, image1rgba.cols, image1rgba.type() } };
			image1rgba.copyTo(cv::Mat{ m_pixelsRgba, cv::Rect{ 0, 0, image1rgba.cols, image1rgba.rows } });
			image2rgbaWarped.copyTo(cv::Mat{ m_pixelsRgba, cv::Rect{ 0, image1rgba.rows, image2rgbaWarped.cols, image2rgbaWarped.rows } });
const auto ms_6 = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);
std::cout << "get_image \t = " << ms_1 << "\n";
std::cout << "det+track \t = " << ms_2 - ms_1 << "\n";
std::cout << "hom+illum \t = " << ms_3 - ms_2 << "\n";
std::cout << "resize    \t = " << ms_4 - ms_3 << "\n";
std::cout << "setMask   \t = " << ms_5 - ms_4 << "\n";
std::cout << "join image\t = " << ms_6 - ms_5 << "\n";
		}
		
std::cout << "total_ms  \t = " << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6) << "\n";
std::cout << "-----------------------------------------------" << std::endl;
		return m_pixelsRgba;
	}

private:
	cv::Mat m_pixelsRgba;
	cv::Mat m_image1rgba;
	cv::Mat m_image2rgba;
	dlib::array2d<dlib::bgr_pixel> m_image2bgrDlib;
	dlib::rectangle m_previousRectangleImage1;
	FramesProducer m_framesProducer;
	DlibFaceTracker m_dlibFaceTracker;
	Landmarks m_landmarksImage2;

	void setMask(cv::Mat & imageWithMask, const cv::Rect & faceLocation) const
	{
		const cv::Size halfWH{ (int)std::round((faceLocation.width) / 2.f),
							   (int)std::round((faceLocation.height) / 2.f) };
		const cv::Point faceCenter{ faceLocation.x + halfWH.width,
									faceLocation.y + halfWH.height };
		const auto cornerLength = std::sqrt(halfWH.width * halfWH.width + halfWH.height * halfWH.height);
		const auto maximum = 0.95 * cornerLength;
		const auto minimum = 0.6 * cornerLength;
		//const auto minimum = std::min(halfWH.width, halfWH.height);
		//const auto maximum = minimum;
		for (auto y = 0; y < imageWithMask.rows; y++)
		{
			const auto yL2Norm = (faceCenter.y - y)*(faceCenter.y - y);
			for (auto x = 0; x < imageWithMask.cols; x++)
			{
				const auto l2Norm = std::sqrt((faceCenter.x - x)*(faceCenter.x - x) + yL2Norm);
				if (l2Norm >= maximum)
					imageWithMask.at<cv::Vec4b>(y, x)[3] = { 0 };
				else if (l2Norm <= minimum)
					imageWithMask.at<cv::Vec4b>(y, x)[3] = { 255 };
				else
					imageWithMask.at<cv::Vec4b>(y, x)[3] = (uchar)std::round(255 * 0.5 * (1 + std::cos(3. * (l2Norm - minimum) / (maximum - minimum))));
			}
		}
	}

	cv::Mat readImageFromFile(const std::string & filePath) const
	{
		// Read image
		cv::Mat pixelsBGR = cv::imread(filePath);
		if (pixelsBGR.empty())
			throw std::runtime_error{ "Failed to load texture image!" };
		return pixelsBGR;
	}

	void padOrCropToFixedSize(cv::Mat & image, cv::Size & outputSize) const
	{
		// Crop image (if required)
		image = { cv::Mat{ image, cv::Rect{ 0, 0, std::min(outputSize.width, image.cols), std::min(outputSize.height, image.rows) } } };

		// Add padding (if required)
		const cv::Size sizeDifference{ outputSize - image.size() };
		if (sizeDifference.width != 0 || sizeDifference.height != 0)
			cv::copyMakeBorder(image, image, 0, std::max(0, sizeDifference.height), 0, std::max(0, sizeDifference.width), cv::BORDER_REPLICATE);
	}
};

#endif // OPEN_CV_PROCESSING_H
