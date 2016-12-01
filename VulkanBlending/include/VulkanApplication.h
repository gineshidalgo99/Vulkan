#ifndef VULKAN_WRAPPER_H
#define VULKAN_WRAPPER_H

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>

#include "VulkanDeleteClassWrapper.h"
//#include "VulkanIteration.h"
#include "VulkanStructs.h"
#include "VulkanUtilities.h"

// To see if there are more layers, uncomment lines after: "// Output all available validation layers" 
const std::vector<const char*> desiredValidationLayers = {
	"VK_LAYER_LUNARG_standard_validation",
	//"VK_LAYER_LUNARG_api_dump",	// It just starts printing out weird things
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_image",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_screenshot",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_GOOGLE_unique_objects",
	//"VK_LAYER_LUNARG_vktrace",	// It needs some kind of client/server connection
	"VK_LAYER_RENDERDOC_Capture",	// It shows fps and allows screenshots
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const auto enableValidationLayers = false;
#else
const auto enableValidationLayers = true;
#endif

const std::vector<Vertex> vertices = {
	// Image 1
	{ { -1.f, -1.f, -0.002f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
	{ { 1.f, -1.f, -0.002f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 0.5f, 1.0f } },
	{ { 1.f, 1.f, -0.002f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 0.5f, 0.0f } },
	{ { -1.f, 1.f, -0.002f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	// Image 2
	{ { -1.f, -1.f, -0.001f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 0.5f, 1.0f } },
	{ { 1.f, -1.f, -0.001f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } },
	{ { 1.f, 1.f, -0.001f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { -1.f, 1.f, -0.001f },{ 0.0f, 0.0f, 0.0f, 0.0f },{ 0.5f, 0.0f } },
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
};

class VulkanApplication
{
	public:
		void run()
		{
			//for (auto i = 0; i < 2; i++)
			//{
			initWindow();
			initVulkan();
			mainLoop();
			//cleanup();
			//}
		}

	private:
		GLFWwindow* window;

		VulkanDeleteClassWrapper<VkInstance> instance{ vkDestroyInstance };
		VulkanDeleteClassWrapper<VkDebugReportCallbackEXT> callback{ instance, VulkanUtilities::DestroyDebugReportCallbackEXT };
		VulkanDeleteClassWrapper<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VulkanDeleteClassWrapper<VkDevice> device{ vkDestroyDevice };

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VulkanDeleteClassWrapper<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VulkanDeleteClassWrapper<VkImageView>> swapChainImageViews;
		std::vector<VulkanDeleteClassWrapper<VkFramebuffer>> swapChainFramebuffers;

		VulkanDeleteClassWrapper<VkRenderPass> renderPass{ device, vkDestroyRenderPass };
		VulkanDeleteClassWrapper<VkDescriptorSetLayout> descriptorSetLayout{ device, vkDestroyDescriptorSetLayout };
		VulkanDeleteClassWrapper<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
		VulkanDeleteClassWrapper<VkPipeline> graphicsPipeline{ device, vkDestroyPipeline };

		VulkanDeleteClassWrapper<VkCommandPool> commandPool{ device, vkDestroyCommandPool };

		VulkanDeleteClassWrapper<VkImage> depthImage{ device, vkDestroyImage };
		VulkanDeleteClassWrapper<VkDeviceMemory> depthImageMemory{ device, vkFreeMemory };
		VulkanDeleteClassWrapper<VkImageView> depthImageView{ device, vkDestroyImageView };

		VulkanDeleteClassWrapper<VkImage> textureImage{ device, vkDestroyImage };
		VulkanDeleteClassWrapper<VkDeviceMemory> textureImageMemory{ device, vkFreeMemory };
		VulkanDeleteClassWrapper<VkImageView> textureImageView{ device, vkDestroyImageView };
		VulkanDeleteClassWrapper<VkSampler> textureSampler{ device, vkDestroySampler };

		VulkanDeleteClassWrapper<VkBuffer> vertexBuffer{ device, vkDestroyBuffer };
		VulkanDeleteClassWrapper<VkDeviceMemory> vertexBufferMemory{ device, vkFreeMemory };
		VulkanDeleteClassWrapper<VkBuffer> indexBuffer{ device, vkDestroyBuffer };
		VulkanDeleteClassWrapper<VkDeviceMemory> indexBufferMemory{ device, vkFreeMemory };

		VulkanDeleteClassWrapper<VkBuffer> uniformStagingBuffer{ device, vkDestroyBuffer };
		VulkanDeleteClassWrapper<VkDeviceMemory> uniformStagingBufferMemory{ device, vkFreeMemory };
		VulkanDeleteClassWrapper<VkBuffer> uniformBuffer{ device, vkDestroyBuffer };
		VulkanDeleteClassWrapper<VkDeviceMemory> uniformBufferMemory{ device, vkFreeMemory };

		VulkanDeleteClassWrapper<VkDescriptorPool> descriptorPool{ device, vkDestroyDescriptorPool };
		VkDescriptorSet descriptorSet;

		std::vector<VkCommandBuffer> commandBuffers;

		VulkanDeleteClassWrapper<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
		VulkanDeleteClassWrapper<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

		//void cleanup()
		//{
		//	renderFinishedSemaphore.cleanup();
		//	imageAvailableSemaphore.cleanup();

		//	descriptorPool.cleanup();

		//	uniformBufferMemory.cleanup();
		//	uniformBuffer.cleanup();
		//	uniformStagingBufferMemory.cleanup();
		//	uniformStagingBuffer.cleanup();

		//	indexBufferMemory.cleanup();
		//	indexBuffer.cleanup();
		//	vertexBufferMemory.cleanup();
		//	vertexBuffer.cleanup();

		//	textureSampler.cleanup();
		//	textureImageView.cleanup();
		//	textureImageMemory.cleanup();
		//	textureImage.cleanup();

		//	depthImageView.cleanup();
		//	depthImageMemory.cleanup();
		//	depthImage.cleanup();

		//	commandPool.cleanup();

		//	graphicsPipeline.cleanup();
		//	pipelineLayout.cleanup();
		//	descriptorSetLayout.cleanup();
		//	renderPass.cleanup();

		//	swapChainFramebuffers.clear();
		//	swapChainImageViews.clear();
		//	swapChain.cleanup();

		//	device.cleanup();

		//	surface.cleanup();
		//	callback.cleanup();
		//	instance.cleanup();
		//}

		void initWindow()
		{
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			// No full screen mode
			//window = glfwCreateWindow(1920, 1080, "Vulkan", nullptr, nullptr);
			// Full screen mode
			const auto monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
			window = glfwCreateWindow(mode->width, mode->height, "Vulkan", monitor, NULL);
			// End no full vs. full screen mode

			glfwSetWindowUserPointer(window, this);
			glfwSetWindowSizeCallback(window, VulkanApplication::onWindowResized);
		}

		void initVulkan()
		{
const auto beginClock = std::chrono::high_resolution_clock::now();
std::cout << "init bottelnecks" << std::endl;
			createInstance();
std::cout << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6) << " ";
			setupDebugCallback();
			createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
			createSwapChain();
			createImageViews();
			createRenderPass();
			createDescriptorSetLayout();
			createGraphicsPipeline();
			createCommandPool();
std::cout << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6) << " ";
			createDepthResources();
			createFramebuffers();
std::cout << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6) << " ";
			createTextureImage();
std::cout << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6) << " ";
			createTextureImageView();
			createTextureSampler();
			createVertexBuffer();
			createIndexBuffer();
			createUniformBuffer();
			createDescriptorPool();
			createDescriptorSet();
			createCommandBuffers();
			createSemaphores();
			updateUniformBuffer();
std::cout << (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6) << std::endl;
std::cout << "end bottelnecks" << std::endl;
		}

		void mainLoop()
		{
			const auto fps = 61;
			const auto millisecondsPerFrame = 1000. / fps;
			while (!glfwWindowShouldClose(window))
			{
				//glfwPollEvents();

				//updateUniformBuffer();
				//drawFrame();

				const auto beginClock = std::chrono::high_resolution_clock::now();

				//updateImage();	// Function to be done, not finished
				drawFrame();

				// Measure performance
				const auto durationMs = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6);

				// Process current events
				// Option a - Just process all current queued events
				const auto durationInnerLoopMs = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6);
				const auto otherEventsTimeMs = (millisecondsPerFrame - durationInnerLoopMs) * 0.5;
				if (otherEventsTimeMs > 0)
					glfwWaitEventsTimeout(otherEventsTimeMs * 1e-3);
				else
					glfwPollEvents();
				const auto durationSoFar = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6);
				const auto timeToSleepMs = -(millisecondsPerFrame - durationSoFar - 1.5);
				if (timeToSleepMs > 0)
					std::this_thread::sleep_for(std::chrono::microseconds{ (int)std::round(1e3 * timeToSleepMs) });
				// Option b - Just process all current queued events
				//glfwPollEvents();	// Process all queued events

				// Loop total time
				const auto loopTotalTimeMs = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginClock).count() * 1e-6);

				//// Debugging times
				////std::cout << durationInnerLoopMs << "\n";
				////std::cout << millisecondsPerFrame << "\n";
				////std::cout << otherEventsTimeMs << "\n";
				////std::cout << durationSoFar << "\n";
				////std::cout << timeToSleepMs << "\n";
				////std::cout << "loop total time ms = " << loopTotalTimeMs << "\n" << std::endl;
				//std::cout << "loop total time ms = " << loopTotalTimeMs << std::endl;
			}

			vkDeviceWaitIdle(device);
		}

		static void onWindowResized(GLFWwindow* window, int width, int height)
		{
			if (width == 0 || height == 0)
				return;

			VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
			app->recreateSwapChain();
		}

		void recreateSwapChain()
		{
			vkDeviceWaitIdle(device);

			createSwapChain();
			createImageViews();
			createRenderPass();
			createGraphicsPipeline();
			createDepthResources();
			createFramebuffers();
			createCommandBuffers();
		}

		void createInstance()
		{
			if (enableValidationLayers && !checkValidationLayerSupport())
				throw std::runtime_error{ "validation layers requested, but not available!" };

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Vulkan Blending";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			auto extensions = getRequiredExtensions();
			createInfo.enabledExtensionCount = VulkanUtilities::sizeTToUint32T(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = VulkanUtilities::sizeTToUint32T(desiredValidationLayers.size());
				createInfo.ppEnabledLayerNames = desiredValidationLayers.data();
			}
			else
				createInfo.enabledLayerCount = 0;

			if (vkCreateInstance(&createInfo, nullptr, instance.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create instance!" };
		}

		void setupDebugCallback()
		{
			if (!enableValidationLayers)
				return;

			VkDebugReportCallbackCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
			createInfo.pfnCallback = VulkanUtilities::debugCallback;

			if (VulkanUtilities::CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to set up debug callback!" };
		}

		void createSurface()
		{
			if (glfwCreateWindowSurface(instance, window, nullptr, surface.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create window surface!" };
		}

		void pickPhysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0)
				throw std::runtime_error{" failed to find GPUs with Vulkan support!" };

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (const auto& device : devices)
			{
				if (isDeviceSuitable(device))
				{
					physicalDevice = device;
					break;
				}
			}

			if (physicalDevice == VK_NULL_HANDLE)
				throw std::runtime_error{" failed to find a suitable GPU!" };
		}

		void createLogicalDevice()
		{
			QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

			float queuePriority = 1.0f;
			for (int queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures = {};

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = VulkanUtilities::sizeTToUint32T(queueCreateInfos.size());

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = VulkanUtilities::sizeTToUint32T(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = VulkanUtilities::sizeTToUint32T(desiredValidationLayers.size());
				createInfo.ppEnabledLayerNames = desiredValidationLayers.data();
			}
			else
				createInfo.enabledLayerCount = 0;

			if (vkCreateDevice(physicalDevice, &createInfo, nullptr, device.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create logical device!" };

			vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
			vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
		}

		void createSwapChain()
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
				imageCount = swapChainSupport.capabilities.maxImageCount;

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
			uint32_t queueFamilyIndices[] = { VulkanUtilities::sizeTToUint32T(indices.graphicsFamily), VulkanUtilities::sizeTToUint32T(indices.presentFamily) };

			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			VkSwapchainKHR oldSwapChain = swapChain;
			createInfo.oldSwapchain = oldSwapChain;

			VkSwapchainKHR newSwapChain;
			if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapChain) != VK_SUCCESS)
				throw std::runtime_error{" failed to create swap chain!" };

			swapChain = newSwapChain;

			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

			swapChainImageFormat = surfaceFormat.format;
			swapChainExtent = extent;
		}

		void createImageViews()
		{
			swapChainImageViews.resize(swapChainImages.size(), VulkanDeleteClassWrapper<VkImageView>{device, vkDestroyImageView});

			for (uint32_t i = 0; i < swapChainImages.size(); i++)
				createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i]);
		}

		void createRenderPass()
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = findDepthFormat();
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subPass = {};
			subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subPass.colorAttachmentCount = 1;
			subPass.pColorAttachments = &colorAttachmentRef;
			subPass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = VulkanUtilities::sizeTToUint32T(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subPass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create render pass!" };
		}

		void createDescriptorSetLayout()
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = 1;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = VulkanUtilities::sizeTToUint32T(bindings.size());
			layoutInfo.pBindings = bindings.data();

			if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, descriptorSetLayout.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create descriptor set layout!" };
		}

		void createGraphicsPipeline()
		{
			const auto vertShaderCode(VulkanUtilities::readFile("shaders/vert.spv"));
			const auto fragShaderCode(VulkanUtilities::readFile("shaders/frag.spv"));

			VulkanDeleteClassWrapper<VkShaderModule> vertShaderModule{ device, vkDestroyShaderModule };
			VulkanDeleteClassWrapper<VkShaderModule> fragShaderModule{ device, vkDestroyShaderModule };
			createShaderModule(vertShaderCode, vertShaderModule);
			createShaderModule(fragShaderCode, fragShaderModule);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();

			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = VulkanUtilities::sizeTToUint32T(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapChainExtent.width;
			viewport.height = (float)swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout };
			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = setLayouts;

			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipelineLayout.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create pipeline layout!" };

			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create graphics pipeline!" };
		}

		void createFramebuffers()
		{
			swapChainFramebuffers.resize(swapChainImageViews.size(), VulkanDeleteClassWrapper<VkFramebuffer>{device, vkDestroyFramebuffer});

			for (size_t i = 0; i < swapChainImageViews.size(); i++)
			{
				std::array<VkImageView, 2> attachments = {
					swapChainImageViews[i],
					depthImageView
				};

				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = VulkanUtilities::sizeTToUint32T(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, swapChainFramebuffers[i].replace()) != VK_SUCCESS)
					throw std::runtime_error{" failed to create framebuffer!" };
			}
		}

		void createCommandPool()
		{
			QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

			if (vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create graphics command pool!" };
		}

		void createDepthResources()
		{
			VkFormat depthFormat = findDepthFormat();

			createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
			createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);

			transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
		{
			for (VkFormat format : candidates)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
					return format;
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
					return format;
			}

			throw std::runtime_error{" failed to find supported format!" };
		}

		VkFormat findDepthFormat() const
		{
			return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
				);
		}

		bool hasStencilComponent(VkFormat format) const
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		cv::Mat readImageFromFile(const std::string & filePath) const
		{
			cv::Mat pixelsRGBA;

			// Read image
			cv::Mat pixelsBGR = cv::imread(filePath);
			if (pixelsBGR.empty())
				throw std::runtime_error{" failed to load texture image!" };
			// BGR to RGBA
			cv::cvtColor(pixelsBGR, pixelsRGBA, CV_BGR2RGBA);

			return pixelsRGBA;
		}

		void setMask(cv::Mat & imageWithMask) const
		{
			for (auto x = 0; x < imageWithMask.cols; x++)
			{
				for (auto y = 0; y < imageWithMask.rows; y++)
				{
					imageWithMask.at<cv::Vec4b>(y, x)[3] = (int)std::round(255.* x / (double)imageWithMask.cols);
				}
			}
		}

		void createTextureImage()
		{
			// Load images
			auto pixelsRGBA1 = readImageFromFile("textures/texture1.jpg");
			auto pixelsRGBA2 = readImageFromFile("textures/texture2.jpg");
			//setMask(pixelsRGBA1);
			setMask(pixelsRGBA2);
			cv::Mat pixelsRGBA{ pixelsRGBA1.rows, pixelsRGBA1.cols + pixelsRGBA2.cols, pixelsRGBA1.type() };
			pixelsRGBA1.copyTo(cv::Mat{ pixelsRGBA, cv::Rect{ 0, 0, pixelsRGBA1.cols, pixelsRGBA1.rows } });
			pixelsRGBA2.copyTo(cv::Mat{ pixelsRGBA, cv::Rect{ pixelsRGBA1.cols, 0, pixelsRGBA2.cols, pixelsRGBA2.rows } });
			//cv::imshow("asdf", pixelsRGBA);
			//cv::waitKey(0);
			// Prepare GPU
			int texWidth, texHeight;
			texWidth = pixelsRGBA.cols;
			texHeight = pixelsRGBA.rows;
			VkDeviceSize imageSize = texWidth * texHeight * 4;

			VulkanDeleteClassWrapper<VkImage> stagingImage{ device, vkDestroyImage };
			VulkanDeleteClassWrapper<VkDeviceMemory> stagingImageMemory{ device, vkFreeMemory };
			createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

			void* data;
			vkMapMemory(device, stagingImageMemory, 0, imageSize, 0, &data);
			memcpy(data, pixelsRGBA.data, VulkanUtilities::sizeTToUint32T(imageSize));
			vkUnmapMemory(device, stagingImageMemory);

			createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

			transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			copyImage(stagingImage, textureImage, texWidth, texHeight);

			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		//		void updateImage()
		//		{
		//			int texWidth, texHeight;
		//			cv::Mat pixelsBGR = cv::imread("textures/texture.jpg");
		//			if (pixelsBGR.empty())
		//				throw std::runtime_error{" failed to load texture image!");
		//			// BGR to RGB
		//			cv::Mat pixelsRGBA;
		//			// RGB to RGBA
		//			cv::cvtColor(pixelsBGR, pixelsRGBA, CV_BGR2RGBA);
		//			texWidth = pixelsBGR.cols;
		//			texHeight = pixelsBGR.rows;
		//			VkDeviceSize imageSize = texWidth * texHeight * 4;
		//
		//			//VulkanDeleteClassWrapper<VkImage> stagingImage{ device, vkDestroyImage };
		//			//VulkanDeleteClassWrapper<VkDeviceMemory> stagingImageMemory{ device, vkFreeMemory };
		//			//createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);
		//
		//			//void* data;
		//			//vkMapMemory(device, stagingImageMemory, 0, imageSize, 0, &data);
		//			//memcpy(data, pixelsRGBA.data, VulkanUtilities::sizeTToUint32T(imageSize);
		//			//vkUnmapMemory(device, stagingImageMemory);
		//
		//			//createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
		//
		//			//transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		//			//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		//			//copyImage(stagingImage, textureImage, texWidth, texHeight);
		//
		//			//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//
		//			//recreateSwapChain();
		//		}

		void createTextureImageView()
		{
			createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);
		}

		void createTextureSampler()
		{
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			if (vkCreateSampler(device, &samplerInfo, nullptr, textureSampler.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create texture sampler!" };
		}

		void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VulkanDeleteClassWrapper<VkImageView>& imageView) const
		{
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = aspectFlags;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &viewInfo, nullptr, imageView.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create texture image view!" };
		}

		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VulkanDeleteClassWrapper<VkImage>& image, VulkanDeleteClassWrapper<VkDeviceMemory>& imageMemory) const
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateImage(device, &imageInfo, nullptr, image.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create image!" };

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, imageMemory.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to allocate image memory!" };

			vkBindImageMemory(device, image, imageMemory, 0);
		}

		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const
		{
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;

			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (hasStencilComponent(format))
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}
			else
				throw std::invalid_argument("unsupported layout transition!");

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
				);

			endSingleTimeCommands(commandBuffer);
		}

		void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height) const
		{
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

			VkImageSubresourceLayers subResource = {};
			subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subResource.baseArrayLayer = 0;
			subResource.mipLevel = 0;
			subResource.layerCount = 1;

			VkImageCopy region = {};
			region.srcSubresource = subResource;
			region.dstSubresource = subResource;
			region.srcOffset = { 0, 0, 0 };
			region.dstOffset = { 0, 0, 0 };
			region.extent.width = width;
			region.extent.height = height;
			region.extent.depth = 1;

			vkCmdCopyImage(
				commandBuffer,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &region
				);

			endSingleTimeCommands(commandBuffer);
		}

		void createVertexBuffer()
		{
			VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

			VulkanDeleteClassWrapper<VkBuffer> stagingBuffer{ device, vkDestroyBuffer };
			VulkanDeleteClassWrapper<VkDeviceMemory> stagingBufferMemory{ device, vkFreeMemory };
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, vertices.data(), VulkanUtilities::sizeTToUint32T(bufferSize));
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

			copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		}

		void createIndexBuffer()
		{
			VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

			VulkanDeleteClassWrapper<VkBuffer> stagingBuffer{ device, vkDestroyBuffer };
			VulkanDeleteClassWrapper<VkDeviceMemory> stagingBufferMemory{ device, vkFreeMemory };
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, indices.data(), VulkanUtilities::sizeTToUint32T(bufferSize));
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

			copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		}

		void createUniformBuffer()
		{
			VkDeviceSize bufferSize = sizeof(UniformBufferObject);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
		}

		void createDescriptorPool()
		{
			std::array<VkDescriptorPoolSize, 2> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = 1;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 1;

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = VulkanUtilities::sizeTToUint32T(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 1;

			if (vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create descriptor pool!" };
		}

		void createDescriptorSet()
		{
			VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;

			if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
				throw std::runtime_error{" failed to allocate descriptor set!" };

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSet;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSet;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, VulkanUtilities::sizeTToUint32T(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanDeleteClassWrapper<VkBuffer>& buffer, VulkanDeleteClassWrapper<VkDeviceMemory>& bufferMemory)
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create buffer!" };

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to allocate buffer memory!" };

			vkBindBufferMemory(device, buffer, bufferMemory, 0);
		}

		VkCommandBuffer beginSingleTimeCommands() const
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			return commandBuffer;
		}

		void endSingleTimeCommands(VkCommandBuffer commandBuffer) const
		{
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(graphicsQueue);

			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		}

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
		{
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

			VkBufferCopy copyRegion = {};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			endSingleTimeCommands(commandBuffer);
		}

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
					return i;

			throw std::runtime_error{" failed to find suitable memory type!" };
		}

		void createCommandBuffers()
		{
			if (commandBuffers.size() > 0)
				vkFreeCommandBuffers(device, commandPool, VulkanUtilities::sizeTToUint32T(commandBuffers.size()), commandBuffers.data());

			commandBuffers.resize(swapChainFramebuffers.size());

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = VulkanUtilities::sizeTToUint32T(commandBuffers.size());

			if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
				throw std::runtime_error{" failed to allocate command buffers!" };

			for (size_t i = 0; i < commandBuffers.size(); i++)
			{
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

				vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = swapChainFramebuffers[i];
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = swapChainExtent;

				std::array<VkClearValue, 2> clearValues = {};
				clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
				clearValues[1].depthStencil = { 1.0f, 0 };

				renderPassInfo.clearValueCount = VulkanUtilities::sizeTToUint32T(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

				VkBuffer vertexBuffers[] = { vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

				vkCmdDrawIndexed(commandBuffers[i], VulkanUtilities::sizeTToUint32T(indices.size()), 1, 0, 0, 0);

				vkCmdEndRenderPass(commandBuffers[i]);

				if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
					throw std::runtime_error{" failed to record command buffer!" };
			}
		}

		void createSemaphores()
		{
			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, imageAvailableSemaphore.replace()) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, renderFinishedSemaphore.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create semaphores!" };
		}

		void updateUniformBuffer()
		{
			//static auto startTime = std::chrono::high_resolution_clock::now();

			//auto currentTime = std::chrono::high_resolution_clock::now();
			//float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

			//UniformBufferObject ubo = {};
			//ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			//ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
			//ubo.proj[1][1] *= -1;
			UniformBufferObject ubo;
			ubo.model = glm::mat4(1.f);
			ubo.view = glm::mat4(1.f);
			ubo.proj = glm::mat4(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);

			void* data;
			vkMapMemory(device, uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, uniformStagingBufferMemory);

			copyBuffer(uniformStagingBuffer, uniformBuffer, sizeof(ubo));
		}

		void drawFrame()
		{
			uint32_t imageIndex;
			VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				recreateSwapChain();
				return;
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
				throw std::runtime_error{ "failed to acquire swap chain image!" };

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

			VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
				throw std::runtime_error{" failed to submit draw command buffer!" };

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(presentQueue, &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
				recreateSwapChain();
			else if (result != VK_SUCCESS)
				throw std::runtime_error{" failed to present swap chain image!" };
		}

		void createShaderModule(const std::vector<char>& code, VulkanDeleteClassWrapper<VkShaderModule>& shaderModule) const
		{
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = (uint32_t*)code.data();

			if (vkCreateShaderModule(device, &createInfo, nullptr, shaderModule.replace()) != VK_SUCCESS)
				throw std::runtime_error{" failed to create shader module!" };
		}

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
		{
			if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
				return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

			for (const auto& availableFormat : availableFormats)
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return availableFormat;

			return availableFormats[0];
		}

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) const
		{
			for (const auto& availablePresentMode : availablePresentModes)
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					return availablePresentMode;

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
		{
			return capabilities.currentExtent;
			//if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			//	return capabilities.currentExtent;
			//else
			//{
			//	VkExtent2D actualExtent = { WIDTH, HEIGHT };

			//	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			//	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			//	return actualExtent;
			//}
		}

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const
		{
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		bool isDeviceSuitable(VkPhysicalDevice device) const
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			QueueFamilyIndices indices = findQueueFamilies(device);

			bool extensionsSupported = checkDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported)
			{
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.isComplete() && extensionsSupported && swapChainAdequate;
		}

		bool checkDeviceExtensionSupport(VkPhysicalDevice device) const
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for (const auto& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			return requiredExtensions.empty();
		}

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const
		{
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					indices.graphicsFamily = i;

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

				if (queueFamily.queueCount > 0 && presentSupport)
					indices.presentFamily = i;

				if (indices.isComplete())
					break;

				i++;
			}

			return indices;
		}

		std::vector<const char*> getRequiredExtensions() const
		{
			std::vector<const char*> extensions;

			unsigned int glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			for (unsigned int i = 0; i < glfwExtensionCount; i++)
				extensions.push_back(glfwExtensions[i]);

			if (enableValidationLayers)
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

			return extensions;
		}

		bool checkValidationLayerSupport() const
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			//// Output all available validation layers
			//for (const auto& layerProperties : availableLayers)
			//	std::cout << layerProperties.layerName << std::endl;

			// Add all desired validation layers
			for (const char* layerName : desiredValidationLayers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
					return false;
			}

			return true;
		}
};

#endif //VULKAN_WRAPPER_H
