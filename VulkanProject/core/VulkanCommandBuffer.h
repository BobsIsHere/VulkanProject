#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanCommandPool;
class VulkanRenderPass;
class VulkanSwapChain;
class GraphicsPipeline;

class VulkanCommandBuffer
{
public:
	VulkanCommandBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, VulkanRenderPass* pRenderPass, VulkanSwapChain* pSwapChain, GraphicsPipeline* pGraphicsPipeline);
	~VulkanCommandBuffer();

	void Create();
	void Record(uint32_t imageIdx);

	VkCommandBuffer GetCommandBuffer() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanCommandPool* m_pVulkanCommandPool;
	VulkanRenderPass* m_pVulkanRenderPass;
	VulkanSwapChain* m_pVulkanSwapChain;
	GraphicsPipeline* m_pGraphicsPipeline;

	VkCommandBuffer m_CommandBuffer;
};