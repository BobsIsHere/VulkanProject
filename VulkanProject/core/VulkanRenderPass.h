#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanSwapChain;

class VulkanRenderPass
{
public:
	VulkanRenderPass(VulkanDevice* pDevice, VulkanSwapChain* pSwapChain);
	~VulkanRenderPass();

	void Create();
	void Cleanup();

	VkRenderPass GetRenderPass() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanSwapChain* m_pVulkanSwapChain;

	VkRenderPass m_RenderPass;

	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};