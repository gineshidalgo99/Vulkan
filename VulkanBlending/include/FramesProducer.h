#ifndef FRAMES_PRODUCER_H
#define FRAMES_PRODUCER_H

#include <chrono>
#include <thread>
#include <opencv2/core/core.hpp>            // cv::Mat
#include <opencv2/highgui/highgui.hpp>      // cv::VideoCapture

class FramesProducer
{
public:
	enum class FrameRotation : unsigned short
	{
		Degrees0 = 0,
		Degrees90,
		Degrees180,
		Degrees270,
		Size
	};
	enum class FrameMirrorMode : unsigned short
	{
		NoMirrored = 0,
		Mirrored,
		Size
	};
	enum class FrameState : unsigned short
	{
		OK = 0,
		Empty,
		Wrong,
		Size
	};

	FramesProducer(const std::string & cameraPath,
		const FrameMirrorMode frameMirrorMode,
		const FrameRotation frameRotation);
	~FramesProducer();
	std::tuple<FrameState, cv::Mat> getValidFrame();
	bool isOpened();
	void release();

private:
	std::chrono::high_resolution_clock::time_point m_disconnectedWebCamClock;
	bool m_cameraPathIsWebcam;
	const std::string m_cameraPath;
	cv::VideoCapture m_videoCapture;
	const FrameMirrorMode m_frameMirrorMode;
	const FrameRotation m_frameRotation;
	int m_width;
	int m_height;

	bool openVideo(const std::string & cameraPath);
	bool openAndSetWebCamResolution(const std::string & cameraPath);
	bool tryToSetResolution();
	cv::Mat getRawFrame();
	void rotateFrame(cv::Mat & frame);
	FrameState validateFrame(cv::Mat & frame);
	std::vector<cv::Point> getAvailableWebCamResolutions();
};

#endif // FRAMES_PRODUCER_H
