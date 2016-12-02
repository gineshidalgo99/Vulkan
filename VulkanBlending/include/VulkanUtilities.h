#ifndef VULKAN_UTILITIES_H
#define VULKAN_UTILITIES_H

#include <exception>// std::runtime_error
#include <fstream>	// std::ifstream
#include <iostream>	// std::cout, std::endl;
#include <limits>	// std::numeric_limits
#include <vector>
#include <stdint.h>	// uint32_t
#include <vulkan/vulkan.h>

class VulkanUtilities
{
	public:
		static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
		static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
		static uint32_t sizeTToUint32T(const size_t value);
		static std::vector<char> readFile(const std::string& filename);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
};

#endif // VULKAN_UTILITIES_H
