#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "ProcessingThread.h"
#include "VulkanApplication.h"

int main()
{
	auto result = EXIT_SUCCESS;

	try
	{
		// Parameters
		const auto fps = 60.;
		const auto microsecondsPerFrame = 1e6 / (fps * 1.05);

		// Variables
		auto sp_finalFrame(std::make_shared<cv::Mat>());
		auto sp_finalFrameMutex(std::make_shared<std::mutex>());
		VulkanApplication vulkanApplication{ microsecondsPerFrame, sp_finalFrame, sp_finalFrameMutex };

		// Main loop
		ProcessingThread processingThread{ microsecondsPerFrame, sp_finalFrame, sp_finalFrameMutex };
		vulkanApplication.run();

		// Stop processing thread
		processingThread.stop();
	}
	catch (const std::runtime_error & e)
	{
		std::cerr << e.what() << " at line " << __LINE__ << " at function " << __FUNCTION__ << std::endl;
		result = { EXIT_FAILURE };
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << " at line " << __LINE__ << " at function " << __FUNCTION__ << std::endl;
		result = { EXIT_FAILURE };
	}

	if (result != EXIT_SUCCESS)
		system("pause");
	return result;
}
