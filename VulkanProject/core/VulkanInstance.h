#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	void Initialize(const char* appName);
	void Cleanup();

	void SetupDebugMessenger();
	VkInstance GetInstance() const;
	const std::vector<const char*>& GetValidationLayers() const;

private:
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;

	const std::vector<const char*> m_ValidationLayers;

	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};