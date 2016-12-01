#ifndef VULKAN_DELETE_CLASS_WRAPPER_H
#define VULKAN_DELETE_CLASS_WRAPPER_H

#include <functional>	// std::function
#include <vulkan/vulkan.h>

template <typename T>
class VulkanDeleteClassWrapper
{
	public:
		// Constructors / destructor
		VulkanDeleteClassWrapper();
		VulkanDeleteClassWrapper(std::function<void(T, VkAllocationCallbacks*)> deletef);
		VulkanDeleteClassWrapper(const VulkanDeleteClassWrapper<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef);
		VulkanDeleteClassWrapper(const VulkanDeleteClassWrapper<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef);
		~VulkanDeleteClassWrapper();

		// Public functions
		T* replace();

		// Operators
		const T* operator &() const
		{
			return &object;
		}
		operator T() const {
			return object;
		}
		void operator=(T rhs)
		{
			if (rhs != object)
			{
				cleanup();
				object = rhs;
			}
		}
		template<typename V>
		bool operator==(V rhs)
		{
			return object == T(rhs);
		}

//	private:
		T object{ VK_NULL_HANDLE };
		std::function<void(T)> deleter;

		// Private functions
		void cleanup();
};





// Definition
template<typename T>
VulkanDeleteClassWrapper<T>::VulkanDeleteClassWrapper() : VulkanDeleteClassWrapper([](T, VkAllocationCallbacks*) {}) {}

template<typename T>
VulkanDeleteClassWrapper<T>::VulkanDeleteClassWrapper(std::function<void(T, VkAllocationCallbacks*)> deletef)
{
	this->deleter = [=](T obj) { deletef(obj, nullptr); };
}

template<typename T>
VulkanDeleteClassWrapper<T>::VulkanDeleteClassWrapper(const VulkanDeleteClassWrapper<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
{
	this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
}

template<typename T>
VulkanDeleteClassWrapper<T>::VulkanDeleteClassWrapper(const VulkanDeleteClassWrapper<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
{
	this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
}

template<typename T>
VulkanDeleteClassWrapper<T>::~VulkanDeleteClassWrapper()
{
	cleanup();
}

template<typename T>
T * VulkanDeleteClassWrapper<T>::replace()
{
	cleanup();
	return &object;
}

template<typename T>
void VulkanDeleteClassWrapper<T>::cleanup()
{
	if (object != VK_NULL_HANDLE)
		deleter(object);
	object = { VK_NULL_HANDLE };
}

#endif //VULKAN_DELETE_CLASS_WRAPPER_H
