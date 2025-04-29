#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "VulkanDescriptorSet.h"

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
	void Cleanup();

	void Record(uint32_t imageIdx, std::vector<VkFramebuffer> swapChainFramebuffers, VertexBuffer* m_pVertexBuffer, IndexBuffer* m_pIndexBuffer, 
		std::vector<VulkanDescriptorSet*> m_pVulkanDescriptorSets, uint32_t currentFrame, std::vector<uint32_t> indices);

	VkCommandBuffer GetCommandBuffer() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanCommandPool* m_pVulkanCommandPool;
	VulkanRenderPass* m_pVulkanRenderPass;
	VulkanSwapChain* m_pVulkanSwapChain;
	GraphicsPipeline* m_pGraphicsPipeline;

	VkCommandBuffer m_CommandBuffer;
};