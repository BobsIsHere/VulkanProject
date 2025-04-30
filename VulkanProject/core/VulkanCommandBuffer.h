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
	VulkanCommandBuffer();
	~VulkanCommandBuffer();

	void Create(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);

	void Reset(VkCommandBufferResetFlags flags = 0) const;
	void Begin(VkCommandBufferUsageFlags flags = 0) const;
	void End() const;

	void Submit(VkQueue queue, const VkSubmitInfo& info, VkFence fence = VK_NULL_HANDLE) const;

	void SetCommandBuffer(VkCommandBuffer commandBuffer);
	VkCommandBuffer GetCommandBuffer() const;

	void Record(uint32_t imageIdx, std::vector<VkFramebuffer> swapChainFramebuffers, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, VulkanRenderPass* pRenderPass,
		VulkanSwapChain* pSwapChain, GraphicsPipeline* pPipeline, std::vector<VulkanDescriptorSet*> m_pVulkanDescriptorSets, 
		uint32_t currentFrame, std::vector<uint32_t> indices);

private:
	VkCommandBuffer m_CommandBuffer;
};