#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class Window;
class UniformBuffer;
class VertexBuffer;
class IndexBuffer;
class VulkanCommandBuffer;
class GraphicsPipeline;
class VulkanDescriptorSet;
class VulkanImage;

class Renderer final
{
public:
	Renderer(VulkanDevice* device, VulkanSwapChain* swapChain, VulkanRenderPass* renderPass, Window* window);
	~Renderer();

	void CreateDepthResources();
	void Cleanup();

	void CreateFrameBuffers();
	void DrawFrame(std::vector<UniformBuffer*> pUniformBuffers, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, std::vector<VulkanCommandBuffer*> pCommandBuffers,
		GraphicsPipeline* pPipeline, std::vector<VulkanDescriptorSet*> pVulkanDescriptorSets, std::vector<uint32_t> indices);

	void CleanupSwapChain();
	void RecreateSwapChain();
	void CreateSyncObjects();

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanSwapChain* m_pVulkanSwapChain;
	VulkanRenderPass* m_pVulkanRenderPass;
	Window* m_pWindow;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	std::vector<VkFramebuffer> m_SwapChainFrameBuffers;

	uint32_t m_CurrentFrame = 0;

	VulkanImage* m_pDepthImage;
};