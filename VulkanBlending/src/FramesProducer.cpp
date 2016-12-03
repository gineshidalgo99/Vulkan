#include <iostream>
#include "ErrorHandler.h"
#include "FramesProducer.h"

// Constructors
FramesProducer::FramesProducer(const std::string & cameraPath,
	const FrameMirrorMode frameMirrorMode,
	const FrameRotation frameRotation) :
	m_frameMirrorMode{ frameMirrorMode },
	m_frameRotation{ frameRotation },
	m_cameraPath{ cameraPath },
	m_width{ 0 },
	m_height{ 0 }
{
	// 1. Load available resolutions
	m_cameraPathIsWebcam = { true };
	try
	{
		openAndSetWebCamResolution(cameraPath);
	}
	catch (const std::exception &)
	{
		m_cameraPathIsWebcam = { false };
		openVideo(cameraPath);
	}
}

FramesProducer::~FramesProducer()
{
	release();
}





// Public functions
std::tuple<FramesProducer::FrameState, cv::Mat> FramesProducer::getValidFrame()
{
	auto frame(getRawFrame());
	auto frameState = validateFrame(frame);
	if (frameState == FrameState::OK)
		rotateFrame(frame);
	else if (!m_cameraPathIsWebcam && frameState == FrameState::Empty)
		openVideo(m_cameraPath);
	return std::make_tuple(frameState, frame);
}

bool FramesProducer::isOpened()
{
	return m_videoCapture.isOpened();
}

void FramesProducer::release()
{
	m_videoCapture.release();
}





// Private functions
bool FramesProducer::openVideo(const std::string & cameraPath)
{
	m_videoCapture = { cv::VideoCapture{ cameraPath } };
	if (m_videoCapture.isOpened())
	{
		auto frame(getRawFrame());
		if (!frame.empty() && frame.cols > 0 && frame.rows > 0)
		{
			m_width = { frame.cols };
			m_height = { frame.rows };
		}
		else
			m_videoCapture.release();
	}

	return m_videoCapture.isOpened();
}

bool FramesProducer::openAndSetWebCamResolution(const std::string & cameraPath)
{
	try
	{
		release();

		// 1. Open and set webcam resolution
		if (cameraPath == "")
			m_videoCapture = { cv::VideoCapture{ 0 } };
		else
			m_videoCapture = { cv::VideoCapture{ std::stoi(cameraPath) } };



		// Set maximum available resolution
		auto webCamIsOpened = m_videoCapture.isOpened();
		if (webCamIsOpened)
		{
			if (tryToSetResolution())
			{
				m_videoCapture.set(CV_CAP_PROP_FPS, 30);
				auto frame(getRawFrame());
				if (!frame.empty() && frame.cols > 0 && frame.rows > 0)
				{
					m_width = { frame.cols };
					m_height = { frame.rows };
				}
				else
					webCamIsOpened = { false };
			}

			else
				webCamIsOpened = { false };
		}

		if (!webCamIsOpened)
		{
			m_videoCapture.release();
			std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
		}
		// 3. WebCam did not open -> exception
		if (!webCamIsOpened)
		{
			ErrorHandler::handle(ErrorHandler::ErrorCode::Warning_00002, __FILE__, __LINE__, __FUNCTION__, "Web cam could not be opened.");
			return false;
		}
		return true;
	}
	catch (const std::exception & e)
	{
		ErrorHandler::handle(ErrorHandler::ErrorCode::CriticalError_00000, __FILE__, __LINE__, __FUNCTION__, e.what());
		return false;
	}
}

bool FramesProducer::tryToSetResolution()
{
	auto foundResolution = false;

	auto availableWebCamResolutions(getAvailableWebCamResolutions());
	for (auto i = 0; i < (int)availableWebCamResolutions.size() && !foundResolution; i++)
	{
		if (i != 0 &&
			(m_videoCapture.get(CV_CAP_PROP_FRAME_WIDTH) < availableWebCamResolutions[i].x
				|| m_videoCapture.get(CV_CAP_PROP_FRAME_WIDTH) < availableWebCamResolutions[i].y))
		{
			continue;
		}
		m_videoCapture.set(CV_CAP_PROP_FRAME_WIDTH, availableWebCamResolutions[i].x);
		m_videoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, availableWebCamResolutions[i].y);
		foundResolution = { m_videoCapture.get(CV_CAP_PROP_FRAME_WIDTH) == availableWebCamResolutions[i].x
			&& m_videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT) == availableWebCamResolutions[i].y };
	}

	return foundResolution;
}

cv::Mat FramesProducer::getRawFrame()
{
	cv::Mat frame;
	m_videoCapture >> frame;

	if (frame.empty())
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto durationMilliseconds = (unsigned int)std::round(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_disconnectedWebCamClock).count() / 1e6);

		// Threshold to avoid trying to reopening the web cam in each iteration (it will slow down the program and user responsivity a lot)
		if (durationMilliseconds > 1500u)      // 15 iterations is around 500 mseconds, 30 iterations is around 1 second
		{
			// Trying to re-open webcam
			if (openAndSetWebCamResolution(m_cameraPath))
				frame = { FramesProducer::getRawFrame() };
			m_disconnectedWebCamClock = { std::chrono::high_resolution_clock::now() };
		}
	}
	else
		m_disconnectedWebCamClock = { std::chrono::high_resolution_clock::now() };

	return frame;
}

void FramesProducer::rotateFrame(cv::Mat & frame)
{
	// Rotate it if desired
	if (m_frameRotation == FrameRotation::Degrees90)
	{
		if (m_frameMirrorMode == FrameMirrorMode::NoMirrored)
		{
			cv::transpose(frame, frame);
			cv::flip(frame, frame, 0);
		}
		else
		{
			cv::transpose(frame, frame);
			cv::flip(frame, frame, -1);
		}
	}
	else if (m_frameRotation == FrameRotation::Degrees180)
	{
		if (m_frameMirrorMode == FrameMirrorMode::NoMirrored)
			cv::flip(frame, frame, -1);
		else
			cv::flip(frame, frame, 0);
	}
	else if (m_frameRotation == FrameRotation::Degrees270)
	{
		if (m_frameMirrorMode == FrameMirrorMode::NoMirrored)
		{
			cv::transpose(frame, frame);
			cv::flip(frame, frame, 1);
		}
		else
			cv::transpose(frame, frame);
	}
	else //if (m_frameRotation == FrameRotation::Degrees0)
		if (m_frameMirrorMode == FrameMirrorMode::Mirrored)
			flip(frame, frame, 1);
}

FramesProducer::FrameState FramesProducer::validateFrame(cv::Mat & frame)
{
	if (frame.empty() || frame.rows == 0 || frame.cols == 0)
	{
		ErrorHandler::handle(ErrorHandler::ErrorCode::Warning_00002, __FILE__, __LINE__, __FUNCTION__, "Empty image");
		return FrameState::Empty;
	}
	else if (m_width != frame.cols || m_height != frame.rows)
	{
		ErrorHandler::handle(ErrorHandler::ErrorCode::Warning_00002, __FILE__, __LINE__, __FUNCTION__, "Wrong image format");
		return FrameState::Wrong;
	}

	return FrameState::OK;
}

std::vector<cv::Point> FramesProducer::getAvailableWebCamResolutions()
{
	return{
		//cv::Point{3840, 2160},     // 4K
		//cv::Point{2048, 1080},     // 2K
		//        cv::Point{1920, 1080},       // HD 1080
		//cv::Point{1280, 1024},     // SXGA
		//cv::Point{1280, 768},      // WXGA
		cv::Point{ 1280, 720 },        // HD 720
		cv::Point{ 800, 600 },         // SVGA
		cv::Point{ 640, 480 },         // VGA
		cv::Point{ (int)std::round(m_videoCapture.get(CV_CAP_PROP_FRAME_WIDTH)),
		(int)std::round(m_videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT)) },
	};
}
