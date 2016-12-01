#include "VulkanUtilities.h"

VkResult VulkanUtilities::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugReportCallbackEXT * pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanUtilities::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks * pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}

uint32_t VulkanUtilities::sizeTToUint32T(const size_t value)
{
	if (value > std::numeric_limits<uint32_t>::max())
		throw std::runtime_error{ "value > std::numeric_limits<uint32_t>::max())" };

	return{ (uint32_t)value };
}

std::vector<char> VulkanUtilities::readFile(const std::string & filename)
{
	std::ifstream file{ filename, std::ios::ate | std::ios::binary };

	if (!file.is_open())
		throw std::runtime_error{ " failed to open file!" };

	const size_t fileSize = sizeTToUint32T(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtilities::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}
