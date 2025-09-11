#pragma once

#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "imgui.h"

#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "VulkanDescriptorSet.h"

class VulkanDevice;
class VulkanCommandPool;
class VulkanRenderContext;
class VulkanSwapChain;
class VulkanImage;
class GraphicsPipeline;
class Model;

class VulkanCommandBuffer final
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

	void Record(uint32_t imageIdx, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, VulkanRenderContext* pRenderContext, VulkanSwapChain* pSwapChain, GraphicsPipeline* pPipeline,
		const std::unique_ptr<Model>& pModel, uint32_t currentFrame, ImDrawData* drawData, VulkanImage* pDepthImage, VulkanDevice* pDevice);

private:
	VkCommandBuffer m_CommandBuffer;
};