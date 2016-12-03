#ifndef OPEN_CV_PROCESSING_H
#define OPEN_CV_PROCESSING_H

#include <opencv2/opencv.hpp>
#include "FramesProducer.h"
#include "Utilities.h"

const auto modeVideo = true;

class OpenCvProcessing
{
public:
	OpenCvProcessing() :
		m_framesProducer{"0", FramesProducer::FrameMirrorMode::NoMirrored, FramesProducer::FrameRotation::Degrees0}
	{
		// Loading face to swap
		if (!modeVideo)
			m_pixelsRGBA1 = { readImageFromFile("textures/texture1.jpg") };
		m_pixelsRGBA2 = { readImageFromFile("textures/texture2.jpg") };
		// To check mask
		//m_pixelsRGBA1.setTo(0);
		//m_pixelsRGBA2.setTo(255);
	}

	cv::Mat getNextImage()
	{
		cv::Mat pixelsRGBA;

		auto frameState = FramesProducer::FrameState::OK;
		if (modeVideo)
			std::tie(frameState, m_pixelsRGBA1) = m_framesProducer.getValidFrame();

		if (frameState == FramesProducer::FrameState::OK && !m_pixelsRGBA1.empty() && !m_pixelsRGBA2.empty())
		{
			// BGR to RGBA
			if (modeVideo)
				cv::cvtColor(m_pixelsRGBA1, m_pixelsRGBA1, CV_BGR2RGBA);

			if (m_pixelsRGBA1.size() != m_pixelsRGBA2.size())
				cv::resize(m_pixelsRGBA2, m_pixelsRGBA2, m_pixelsRGBA1.size(), 0.0, 0.0, CV_INTER_AREA);

			setMask(m_pixelsRGBA2, { m_pixelsRGBA2.cols / 2 - 100, m_pixelsRGBA2.rows / 2 - 100, 200, 200 });
			pixelsRGBA = cv::Mat{ m_pixelsRGBA1.rows + m_pixelsRGBA2.rows, m_pixelsRGBA1.cols, m_pixelsRGBA1.type() };
			m_pixelsRGBA1.copyTo(cv::Mat{ pixelsRGBA, cv::Rect{ 0, 0, m_pixelsRGBA1.cols, m_pixelsRGBA1.rows } });
			m_pixelsRGBA2.copyTo(cv::Mat{ pixelsRGBA, cv::Rect{ 0, m_pixelsRGBA1.rows, m_pixelsRGBA2.cols, m_pixelsRGBA2.rows } });
		}

		return pixelsRGBA;
	}

private:
	cv::Mat m_pixelsRGBA1;
	cv::Mat m_pixelsRGBA2;
	FramesProducer m_framesProducer;

	void setMask(cv::Mat & imageWithMask, const cv::Rect & faceLocation) const
	{
		const cv::Point faceCenter{ (int)std::round((faceLocation.x + faceLocation.width) / 2.f),
									(int)std::round((faceLocation.y + faceLocation.height) / 2.f) };
		const auto usingFace = true;
		if (usingFace)
		{
			const auto maximum = 1.25 * std::sqrt(faceLocation.width * faceLocation.width + faceLocation.height * faceLocation.height);
			const auto minimum = 0.7 * std::min(faceLocation.width, faceLocation.height);
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
		else
		{
			if (true)
				//if (rand() % 2 == 1)
				for (auto y = 0; y < imageWithMask.rows; y++)
					for (auto x = 0; x < imageWithMask.cols; x++)
						imageWithMask.at<cv::Vec4b>(y, x)[3] = (int)std::round(255.* x / (double)imageWithMask.cols);
			else
				for (auto y = 0; y < imageWithMask.rows; y++)
					for (auto x = 0; x < imageWithMask.cols; x++)
						imageWithMask.at<cv::Vec4b>(y, x)[3] = 1;
		}
	}

	cv::Mat readImageFromFile(const std::string & filePath) const
	{
		cv::Mat pixelsRGBA;

		// Read image
		cv::Mat pixelsBGR = cv::imread(filePath);
		if (pixelsBGR.empty())
			throw std::runtime_error{ " failed to load texture image!" };
		// BGR to RGBA
		cv::cvtColor(pixelsBGR, pixelsRGBA, CV_BGR2RGBA);

		return pixelsRGBA;
	}
};

#endif // OPEN_CV_PROCESSING_H
