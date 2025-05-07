#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanImage;

class FramebufferManager
{
public:
	FramebufferManager() = default;
	~FramebufferManager() = default;

	void CreateFramebuffers(VulkanDevice* pDevice, VulkanSwapChain* pSwapChain, VulkanRenderPass* pRenderpass, VulkanImage* pDepthImage);
	void Cleanup(VulkanDevice* pDevice);
	const std::vector<VkFramebuffer>& GetFramebuffers() const;

private:
	std::vector<VkFramebuffer> m_Framebuffers;
};