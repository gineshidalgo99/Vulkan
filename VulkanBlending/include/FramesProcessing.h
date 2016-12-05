#ifndef OPEN_CV_PROCESSING_H
#define OPEN_CV_PROCESSING_H

#include <opencv2/opencv.hpp>
#include "DlibFaceTracker.h"
#include "DlibUtilities.h"
#include "FaceWarp.h"
#include "FramesProducer.h"
#include "Utilities.h"

const auto MODE_VIDEO = true;
const auto MODE_DEMO = true;

class FramesProcessing
{
public:
	FramesProcessing() :
		m_dlibFaceTracker{ "../model/shape_predictor_68_face_landmarks.dat" },
		m_framesProducer{(MODE_VIDEO ? "0" : "9"), FramesProducer::FrameMirrorMode::NoMirrored, FramesProducer::FrameRotation::Degrees0},
		m_demoClock{ std::chrono::high_resolution_clock::now() }
	{
		// Loading face to swap
		// Image 1 (if no video)
		if (!MODE_VIDEO)
			m_image1rgba = { readImageFromFile("textures/textureT.jpg") };

		// Image 2
		prepareImage("textures/texture.jpg", m_image2rgba, m_landmarksImage2);
		if (MODE_VIDEO)
		{
			prepareImage("textures/textures/1.jpg", m_images2rgba[0], m_landmarksImages2[0]);
			prepareImage("textures/textures/2.jpg", m_images2rgba[1], m_landmarksImages2[1]);
			prepareImage("textures/textures/3.jpg", m_images2rgba[2], m_landmarksImages2[2]);
		}

		// Images 1 & 2 - RGB to RGBA
		if (!MODE_VIDEO)
			cv::cvtColor(m_image1rgba.clone(), m_image1rgba, CV_BGR2RGBA);

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
		// Video
		if (MODE_VIDEO)
			std::tie(frameState, image1rgba) = m_framesProducer.getValidFrame();
		// Image
		else
		{
			// cv::Mat image
			cv::cvtColor(m_image1rgba, image1rgba, CV_RGBA2BGR);
			// Dlib image
			dlibImage1Bgr = { DlibUtilities::cvMatToDLibBgr(image1rgba) };
		}

		auto offsetX = 0;
		auto offsetY = 0;
		if (frameState == FramesProducer::FrameState::OK && !image1rgba.empty())
		{
			// BGR cv::Mat -> dlib::array2d
			const auto offsetXLimit = (int)std::round(image1rgba.cols * 0.15);
			const auto offsetYLimit = (int)std::round(image1rgba.rows * 0.15);
			const auto offsetWLimit = image1rgba.cols - 2 * offsetXLimit;
			const auto offsetHLimit = image1rgba.rows - 2 * offsetYLimit;
			cv::Rect faceEstimatedRect;
			if (m_previousFaceRect1 != dlib::rectangle{})
			{
				const auto previousWH = (int)std::round(m_previousFaceRect1.right() - m_previousFaceRect1.left());
				offsetX = (int)std::round(m_previousFaceRect1.left() - previousWH * 0.1);
				offsetY = (int)std::round(m_previousFaceRect1.top() - previousWH * 0.1);
				const auto offsetW = (int)std::round(previousWH * 1.2f);
				const auto offsetH = offsetW;
				if (offsetX < offsetXLimit || offsetY < offsetYLimit)
					m_previousFaceRect1 = { dlib::rectangle{} };
				else
					faceEstimatedRect = { cv::Rect{ offsetX, offsetY, (int)std::round(previousWH * 1.2f), (int)std::round(previousWH * 1.2f) } };
			}
			if (m_previousFaceRect1 == dlib::rectangle{})
			{
				offsetX = offsetXLimit;
				offsetY = offsetYLimit;
				faceEstimatedRect = { cv::Rect{ offsetX, offsetY, offsetWLimit, offsetHLimit } };
			}
			dlibImage1Bgr = { DlibUtilities::cvMatToDLibBgr(cv::Mat{image1rgba, faceEstimatedRect }) };

			// image1rgba BGR to RGBA
			if (image1rgba.channels() == 3)
				cv::cvtColor(image1rgba, image1rgba, CV_BGR2RGBA);
		}
		// Fix image
		else
			image1rgba = { m_image1rgba.clone() };
const auto get_image = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);

		if (frameState == FramesProducer::FrameState::OK && !image1rgba.empty() && !m_image2rgba.empty())
		{
			// Face landmarks tracking
			const auto previousLandmarksFound = (m_previousFaceRect1 != dlib::rectangle{});
			bool landmarks1Found;
			Landmarks landmarks1;
			std::tie(landmarks1Found, landmarks1, m_previousFaceRect1) = m_dlibFaceTracker.getFaceLandmarks(dlibImage1Bgr, offsetX, offsetY);
const auto det_track = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6) ;

			// Change image to swap
			if (MODE_DEMO && landmarks1Found && !previousLandmarksFound)
				changeImage2IfTimePast();

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
			//image2rgbaWarped = m_image2rgba;
const auto hom_illum = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);

			// size(Image2warped) = size(image1)
			if (image1rgba.size() != image2rgbaWarped.size())
				padOrCropToFixedSize(image2rgbaWarped, image1rgba.size());
				//cv::resize(image2rgbaWarped, image2rgbaWarped, image1rgba.size(), 0.0, 0.0, CV_INTER_AREA);	// Resize not valid -> it would change face location
const auto resize = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);

			setMask(image1rgba, faceRect);
const auto set_mask = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);
			if (m_pixelsRgba.empty())
				m_pixelsRgba = { cv::Mat{ image1rgba.rows + image2rgbaWarped.rows, image1rgba.cols, image1rgba.type() } };
			image2rgbaWarped.copyTo(cv::Mat{ m_pixelsRgba, cv::Rect{ 0, 0, image2rgbaWarped.cols, image2rgbaWarped.rows } });
			image1rgba.copyTo(cv::Mat{ m_pixelsRgba, cv::Rect{ 0, image2rgbaWarped.rows, image1rgba.cols, image1rgba.rows } });
const auto join_image = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);
std::cout << "get_image \t = " << get_image << "\n";
std::cout << "det+track \t = " << det_track - get_image << "\n";
std::cout << "hom+illum \t = " << hom_illum - det_track << "\n";
std::cout << "resize    \t = " << resize - hom_illum << "\n";
std::cout << "set_mask   \t = " << set_mask - resize << "\n";
std::cout << "join image\t = " << join_image - set_mask << "\n";
		}
		
std::cout << "total_ms  \t = " << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6) << "\n";
std::cout << "-----------------------------------------------" << std::endl;
		return m_pixelsRgba;
	}

private:
	cv::Mat m_pixelsRgba;
	cv::Mat m_image1rgba;
	cv::Mat m_image2rgba;
	Landmarks m_landmarksImage2;
	dlib::rectangle m_previousFaceRect1;
	FramesProducer m_framesProducer;
	DlibFaceTracker m_dlibFaceTracker;
	// For demo
	std::array<cv::Mat, 3> m_images2rgba;
	std::array<Landmarks, 3> m_landmarksImages2;
	int counter;
	std::chrono::high_resolution_clock::time_point m_demoClock;

	void changeImage2IfTimePast()
	{
		//// Option a - Every X seconds
		//const auto everyXSeconds = 4;
		//const auto timePastEveryXSeconds = (int)std::round(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_demoClock).count() * 1e-9 / everyXSeconds);
		//counter = timePastEveryXSeconds % m_images2rgba.size();
		// Option b - Every time tracker is recovered
		counter = (counter+1) % m_images2rgba.size();

		m_image2rgba = { m_images2rgba[counter] };
		m_landmarksImage2 = { m_landmarksImages2[counter] };
	}

	void setMask(cv::Mat & imageWithMask, const cv::Rect & faceLocation) const
	{
		const cv::Size halfWH{ (int)std::round((faceLocation.width) / 2.f),
							   (int)std::round((faceLocation.height) / 2.f) };
		const cv::Point faceCenter{ faceLocation.x + halfWH.width,
									faceLocation.y + halfWH.height };
		const auto cornerLength = std::sqrt(halfWH.width * halfWH.width + halfWH.height * halfWH.height);
		const auto maximum = 0.9 * cornerLength;
		const auto minimum = 0.6 * cornerLength;
		//// For debugging: hard border, without gradient
		//const auto minimum = std::min(halfWH.width, halfWH.height);
		//const auto maximum = minimum;

		// Only apply to region close to faceLocation
		const auto x = std::max(0, (int)std::round(faceCenter.x - maximum));
		const auto y = std::max(0, (int)std::round(faceCenter.y - maximum));
		const auto width = std::min(imageWithMask.cols, (int)std::round(faceCenter.x + maximum - x));
		const auto height = std::min(imageWithMask.rows, (int)std::round(faceCenter.y + maximum - y));
		cv::Mat imageWithMaskCropped{ imageWithMask, cv::Rect{ x, y, width, height } };

		// Update faceCenterOffset with the offset
		const cv::Point faceCenterOffset = {cv::Point{ faceCenter.x - x,
													   faceCenter.y - y } };

		//// Debugging - original
		//cv::Mat imageWithMaskCropped{ imageWithMask };
		//const cv::Point faceCenterOffset = {cv::Point{ faceCenter.x,
		//											   faceCenter.y } };

		// Loop
		for (auto y = 0; y < imageWithMaskCropped.rows; y++)
		{
			const auto yL2Norm = (faceCenterOffset.y - y)*(faceCenterOffset.y - y);
			for (auto x = 0; x < imageWithMaskCropped.cols; x++)
			{
				const auto l2Norm = std::sqrt((faceCenterOffset.x - x)*(faceCenterOffset.x - x) + yL2Norm);
				if (l2Norm <= minimum)
					imageWithMaskCropped.at<cv::Vec4b>(y, x)[3] = { 0 }; 
				else if (l2Norm < maximum)
					//imageWithMaskCropped.at<cv::Vec4b>(y, x)[3] = { 0 };
					imageWithMaskCropped.at<cv::Vec4b>(y, x)[3] = { (uchar)std::round(255 * (1 - 0.5 * (1 + std::cos(3. * (l2Norm - minimum) / (maximum - minimum))))) };
				// By default values are 255
				//else
					//imageWithMaskCropped.at<cv::Vec4b>(y, x)[3] = { 255 };
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

	void prepareImage(const std::string & filePath, cv::Mat & image, Landmarks & landmarks)
	{
		image = { readImageFromFile(filePath) };
		// RGB cv::Mat -> dlib::array2d
		const auto image2bgrDlib = DlibUtilities::cvMatToDLibBgr(image);
		// Landmarks image 2
		bool landmarks2Found;
		dlib::rectangle faceRect2;
		std::tie(landmarks2Found, landmarks, faceRect2) = m_dlibFaceTracker.getFaceLandmarks(image2bgrDlib);
		// Images 1 & 2 - RGB to RGBA
		cv::cvtColor(image, image, CV_BGR2RGBA);
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

	void fixAspectRatio(cv::Mat & image) const
	{
		const auto fullHdAndHdAspectRatio = 16. / 9.;
		const auto width = std::max(image.cols, (int)std::round(fullHdAndHdAspectRatio * image.rows));
		const auto height = std::max(image.rows, (int)std::round(1 / fullHdAndHdAspectRatio * image.cols));
		padOrCropToFixedSize(image, cv::Size{ width, height });
	}
};

#endif // OPEN_CV_PROCESSING_H
