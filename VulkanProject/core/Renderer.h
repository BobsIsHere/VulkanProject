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

class Renderer final
{
public:
	Renderer(VulkanDevice* device, VulkanSwapChain* swapChain, VulkanRenderPass* renderPass, Window* window);
	~Renderer();

	void Cleanup();

	void CreateFrameBuffers();
	void DrawFrame(std::vector<UniformBuffer*> pUniformBuffers, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, std::vector<VulkanCommandBuffer*> pCommandBuffers,
		GraphicsPipeline* pPipeline, std::vector<VulkanDescriptorSet*> pVulkanDescriptorSets, std::vector<uint32_t> indices);

	void CleanupSwapChain();
	void RecreateSwapChain();
	void CreateSyncObjects();
	void CreateDepthResources();

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

	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};