#ifndef PROCESSING_THREAD_H
#define PROCESSING_THREAD_H

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <opencv2/opencv.hpp>
#include "FramesProcessing.h"
#include "Utilities.h"

class ProcessingThread
{
public:
	ProcessingThread(const double microsecondsPerFrame, const std::shared_ptr<cv::Mat> & finalFrameSharedPtr, const std::shared_ptr<std::mutex> & finalFrameMutexSharedPtr) :
		m_microsecondsPerFrame{ microsecondsPerFrame },
		sp_finalFrame{ finalFrameSharedPtr },
		sp_finalFrameMutex{ finalFrameMutexSharedPtr }
	{
		m_thread = { std::thread{&ProcessingThread::threadFunction, this} };
	}
	~ProcessingThread()
	{
		stop();
	}
	void stop()
	{
		closeAndJoinWorkThread();
	}

private:
	const double m_microsecondsPerFrame;
	FramesProcessing m_framesProcessing;
	std::shared_ptr<cv::Mat> sp_finalFrame;
	std::shared_ptr<std::mutex> sp_finalFrameMutex;
	std::thread m_thread;
	std::atomic<bool> m_stopAndCloseThread;

	void threadFunction()
	{
		while (!m_stopAndCloseThread)
		{
			auto clockIteration = std::chrono::high_resolution_clock::now();

			auto pixelsRGBA(m_framesProcessing.getNextImage());

			if (!pixelsRGBA.empty())
			{
				const std::lock_guard<std::mutex> lock(*sp_finalFrameMutex);
				if (sp_finalFrame->empty())
					std::swap(*sp_finalFrame, pixelsRGBA);
			}

			const auto inverseFpsUs = Utilities::sleepGraphicsThreadIfRequired(m_microsecondsPerFrame, clockIteration);
//std::cout << "2nd Thread = " << inverseFpsUs / 1e3 << "ms" << std::endl;
			inverseFpsUs;
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

#endif // PROCESSING_THREAD_H
