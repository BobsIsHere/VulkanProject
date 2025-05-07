#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "Window.h"
#include "VulkanDevice.h"

class VulkanSwapChain final
{
public:
	VulkanSwapChain(Window* pWindow, VulkanDevice* pDevice);
	~VulkanSwapChain();

	void Create();
	void Cleanup();

	void CreateImageViews();

	VkSwapchainKHR GetSwapChain() const;
	std::vector<VkImage> GetSwapChainImages() const;
	VkFormat GetSwapChainImageFormat() const;
	VkExtent2D GetSwapChainExtent() const;
	std::vector<VkImageView> GetSwapChainImageViews() const;

private:
	Window* m_pWindow;
	VulkanDevice* m_pVulkanDevice;

	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImageView> m_SwapChainImageViews;

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};