#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <optional>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <vector>
#include <chrono>
#include <array>
#include <set>

#include "utils/utils.h"
#include "utils/CommandUtils.h"

#include "core/Window.h"
#include "core/VulkanInstance.h"
#include "core/VulkanDevice.h"
#include "core/VulkanSwapChain.h"
#include "core/VulkanRenderContext.h"
#include "core/VulkanCommandPool.h"
#include "core/VulkanCommandBuffer.h"
#include "core/VulkanDescriptorPool.h"
#include "core/VulkanDescriptorSet.h"
#include "core/Renderer.h"
#include "core/Model.h"
#include "core/Texture.h"
#include "core/Camera.h"

#include "pipelines/GraphicsPipeline.h"

#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "buffers/UniformBuffer.h"

class VulkanProject final
{
public:
    void Run();

private:
    void InitVulkan();
    void MainLoop();
    void CleanupVulkan();

    void InitImGui();
	void MainLoopImGui();

    std::unique_ptr<Window> m_pWindow;
    std::unique_ptr<VulkanInstance> m_pVulkanInstance;
    std::unique_ptr<VulkanDevice> m_pVulkanDevice;
    std::unique_ptr<VulkanSwapChain> m_pVulkanSwapChain;
    std::unique_ptr<VulkanRenderContext> m_pVulkanRenderContext;
    std::unique_ptr<GraphicsPipeline> m_pGraphicsPipeline;
    std::unique_ptr<VulkanCommandPool> m_pVulkanCommandPool;

    std::vector<std::unique_ptr<VulkanCommandBuffer>> m_pVulkanCommandBuffers;
    std::unique_ptr<VulkanDescriptorPool> m_pVulkanDescriptorPool;
    std::vector<std::unique_ptr<VulkanDescriptorSet>> m_pVulkanDescriptorSets;
    std::unique_ptr<Renderer> m_pRenderer;
    std::unique_ptr<Model> m_pSponzaModel;
    std::unique_ptr<Texture> m_pSponzaTexture;
    std::unique_ptr<Texture> m_pCurtainBTexture;

    std::unique_ptr<VertexBuffer> m_pVertexBuffer;
    std::unique_ptr<IndexBuffer> m_pIndexBuffer;
    std::vector<std::unique_ptr<UniformBuffer>> m_pUniformBuffers;

	std::unique_ptr<Camera> m_pCamera;

    VkDescriptorPool m_ImGuiPool;
	VkRenderPass m_ImGuiRenderPass;
};