#ifndef OPEN_CV_PROCESSING_H
#define OPEN_CV_PROCESSING_H

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <opencv2/opencv.hpp>
#include "Utilities.h"

class OpenCvProcessing
{
public:
	OpenCvProcessing(const double microsecondsPerFrame, const std::shared_ptr<cv::Mat> & finalFrameSharedPtr, const std::shared_ptr<std::mutex> & finalFrameMutexSharedPtr) :
		m_microsecondsPerFrame{ microsecondsPerFrame },
		sp_finalFrame{ finalFrameSharedPtr },
		sp_finalFrameMutex{ finalFrameMutexSharedPtr }
	{
		// Load images
		m_pixelsRGBA1 = { readImageFromFile("textures/texture1.jpg") };
		m_pixelsRGBA2 = { readImageFromFile("textures/texture2.jpg") };

		m_thread = { std::thread(&OpenCvProcessing::threadFunction, this) };
	}
	~OpenCvProcessing()
	{
		closeAndJoinWorkThread();
	}

private:
	const double m_microsecondsPerFrame;
	cv::Mat m_pixelsRGBA1;
	cv::Mat m_pixelsRGBA2;
	std::shared_ptr<cv::Mat> sp_finalFrame;
	std::shared_ptr<std::mutex> sp_finalFrameMutex;
	std::thread m_thread;
	std::atomic<bool> m_stopAndCloseThread;

	void threadFunction()
	{
		while (!m_stopAndCloseThread)
		{
			auto clockIteration = std::chrono::high_resolution_clock::now();

			if (!m_pixelsRGBA1.empty() && !m_pixelsRGBA2.empty())
			{
				setMask(m_pixelsRGBA2);
				cv::Mat pixelsRGBA{ m_pixelsRGBA1.rows + m_pixelsRGBA2.rows, m_pixelsRGBA1.cols, m_pixelsRGBA1.type() };
				m_pixelsRGBA1.copyTo(cv::Mat{ pixelsRGBA, cv::Rect{ 0, 0, m_pixelsRGBA1.cols, m_pixelsRGBA1.rows } });
				m_pixelsRGBA2.copyTo(cv::Mat{ pixelsRGBA, cv::Rect{ 0, m_pixelsRGBA1.rows, m_pixelsRGBA2.cols, m_pixelsRGBA2.rows } });

				const std::lock_guard<std::mutex> lock(*sp_finalFrameMutex);
				if (sp_finalFrame->empty())
					std::swap(*sp_finalFrame, pixelsRGBA);
			}

			const auto inverseFpsUs = Utilities::sleepGraphicsThreadIfRequired(m_microsecondsPerFrame, clockIteration);
//std::cout << "2nd Thread = " << inverseFpsUs / 1e3 << "ms" << std::endl;
			inverseFpsUs;
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

	void setMask(cv::Mat & imageWithMask) const
	{
		if (true)
		//if (rand() % 2 == 1)
		{
			for (auto y = 0; y < imageWithMask.rows; y++)
			{
				for (auto x = 0; x < imageWithMask.cols; x++)
				{
					imageWithMask.at<cv::Vec4b>(y, x)[3] = (int)std::round(255.* x / (double)imageWithMask.cols);
				}
			}
		}
		else
		{
			for (auto y = 0; y < imageWithMask.rows; y++)
			{
				for (auto x = 0; x < imageWithMask.cols; x++)
				{
					imageWithMask.at<cv::Vec4b>(y, x)[3] = 1;
				}
			}
		}
	}
	void closeAndJoinWorkThread()
	{
		// Pause and later stop work thread
		m_stopAndCloseThread = { true };
		if (m_thread.joinable())
			m_thread.join();
	}
};

#endif // OPEN_CV_PROCESSING_H
