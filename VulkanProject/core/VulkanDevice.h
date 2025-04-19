#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "utils/utils.h"
#include "VulkanInstance.h"
#include "Window.h"

class VulkanDevice
{
public:
	VulkanDevice(VulkanInstance* pInstance, Window* pWindow);
	~VulkanDevice();

	void Cleanup();

	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	VkDevice GetDevice() const;
	VkPhysicalDevice GetPhysicalDevice() const;
	VkQueue GetGraphicsQueue() const;
	VkQueue GetPresentQueue() const;
	VkSurfaceKHR GetSurface() const;

private:
	VulkanInstance* m_pVulkanInstance;
	Window* m_pWindow;

	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;
	VkSurfaceKHR m_Surface;

	const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
};