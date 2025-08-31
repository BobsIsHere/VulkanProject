#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanSwapChain;
class VulkanImage;

class VulkanRenderContext final
{
public:
	VulkanRenderContext(VulkanDevice* pDevice, VulkanSwapChain* pSwapChain);
	~VulkanRenderContext();

	void BuildRenderingInfo(uint32_t imageIdx, VulkanImage* pDepthImage, VkRenderingInfo& outRenderInfo,
		VkRenderingAttachmentInfo& outColorAttachment, VkRenderingAttachmentInfo& outDepthAttachment) const;

	VkFormat GetColorFormat() const;
	VkFormat GetDepthFormat() const;

private:
	VulkanDevice* m_pDevice;
	VulkanSwapChain* m_pSwapChain;

	VkFormat m_ColorFormat;
	VkFormat m_DepthFormat;
};