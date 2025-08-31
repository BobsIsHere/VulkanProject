#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "core/VulkanCommandBuffer.h"

class VulkanDevice;
class VulkanSwapChain;
class VulkanRenderContext;
class Window;
class Camera;
class UniformBuffer;
class VertexBuffer;
class IndexBuffer;
class GraphicsPipeline;
class VulkanDescriptorSet;
class VulkanImage;
class VulkanCommandPool;

class Renderer final
{
public:
	Renderer(VulkanDevice* device, VulkanSwapChain* swapChain, VulkanRenderContext* renderPass, Window* window, Camera* pCamera);
	~Renderer();

	void CreateDepthResources();
	void Cleanup();

	void DrawFrame(std::vector<std::unique_ptr<UniformBuffer>>& pUniformBuffers, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, 
		std::vector<std::unique_ptr<VulkanCommandBuffer>>& pCommandBuffers, VulkanCommandPool* pCommandPool, GraphicsPipeline* pPipeline, 
		std::vector<std::unique_ptr<VulkanDescriptorSet>>& pVulkanDescriptorSets, std::vector<uint32_t> indices, ImDrawData* drawData, VulkanImage* pDepthImage);

	void CleanupSwapChain();
	void RecreateSwapChain();
	void CreateSyncObjects();

	VulkanImage* GetDepthImage() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanSwapChain* m_pVulkanSwapChain;
	VulkanRenderContext* m_pVulkanRenderContext;
	Window* m_pWindow;
	Camera* m_pCamera;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;

	uint32_t m_CurrentFrame = 0;

	VulkanImage* m_pSwapChainImage;
	VulkanImage* m_pDepthImage;
};