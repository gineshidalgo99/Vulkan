#ifndef FRAMES_PROCESSING_H
#define FRAMES_PROCESSING_H

#include <opencv2/opencv.hpp>
#include "Utilities.h"

const std::string TEXTURES_PATH{ "../models/textures/" };

class FramesProcessing
{
public:
	FramesProcessing()
	{
		// Loading face to swap
		// Image 1
		m_image1rgba = { prepareImage(TEXTURES_PATH + "texture1.jpg") };

		// Image 2
		m_image2rgba = { prepareImage(TEXTURES_PATH + "texture2.jpg") };
		cv::resize(m_image2rgba, m_image2rgba, m_image1rgba.size());
		setMask(m_image2rgba);

		// To check mask
		//m_image1rgba.setTo(0);
		//m_image2rgba.setTo(255);
	}

	cv::Mat getNextImage()
	{
//const auto clockIteration = std::chrono::high_resolution_clock::now();

		// Final image
		cv::Mat pixelsRgba{ m_image1rgba.rows + m_image2rgba.rows, m_image1rgba.cols, m_image1rgba.type() };

		m_image1rgba.copyTo(cv::Mat{ pixelsRgba, cv::Rect{ 0, 0, m_image1rgba.cols, m_image1rgba.rows } });
		m_image2rgba.copyTo(cv::Mat{ pixelsRgba, cv::Rect{ 0, m_image1rgba.rows, m_image2rgba.cols, m_image2rgba.rows } });
		
//const auto totalTimeMs = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - clockIteration).count() * 1e-6);
//std::cout << "ms = " << totalTimeMs << "\t fps = " << 1e3 * 10 / std::round(10* totalTimeMs) << "\n";
		return pixelsRgba;
	}

private:
	cv::Mat m_image1rgba;
	cv::Mat m_image2rgba;

	void setMask(cv::Mat & imageWithMask) const
	{
		// Loop
		for (auto y = 0; y < imageWithMask.rows; y++)
			for (auto x = 0; x < imageWithMask.cols; x++)
				imageWithMask.at<cv::Vec4b>(y, x)[3] = { (uchar)std::round(255. * std::sin(3.14159265359 / 2. * x / (double)imageWithMask.rows)) };
	}

	cv::Mat readImageFromFile(const std::string & filePath) const
	{
		// Read image
		cv::Mat pixelsBGR = cv::imread(filePath);
		if (pixelsBGR.empty())
			throw std::runtime_error{ "Failed to load texture image!" };
		return pixelsBGR;
	}

	cv::Mat prepareImage(const std::string & filePath)
	{
		// Read image
		auto image( readImageFromFile(filePath) );
		// RGB to RGBA
		cv::cvtColor(image, image, CV_BGR2RGBA);
		// Return image
		return image;
	}
};

#endif // FRAMES_PROCESSING_H
