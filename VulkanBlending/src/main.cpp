#include "VulkanApplication.h"

int main()
{
	auto result = EXIT_SUCCESS;
	VulkanApplication vulkanApplication;

	try
	{
		vulkanApplication.run();
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
