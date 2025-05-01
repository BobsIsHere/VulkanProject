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

#include "core/Window.h"
#include "core/VulkanInstance.h"
#include "core/VulkanDevice.h"
#include "core/VulkanSwapChain.h"
#include "core/VulkanRenderPass.h"
#include "core/VulkanCommandPool.h"
#include "core/VulkanCommandBuffer.h"
#include "core/VulkanDescriptorPool.h"
#include "core/VulkanDescriptorSet.h"
#include "core/Renderer.h"
#include "core/Model.h"
#include "core/Texture.h"

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
    void CleanupResources();

    // Own Member Variables
    Window* m_pWindow;
    VulkanInstance* m_pVulkanInstance;
    VulkanDevice* m_pVulkanDevice;
    VulkanSwapChain* m_pVulkanSwapChain;
    VulkanRenderPass* m_pVulkanRenderPass;
    GraphicsPipeline* m_pGraphicsPipeline;
    VulkanCommandPool* m_pVulkanCommandPool;
    std::vector<VulkanCommandBuffer*> m_pVulkanCommandBuffers;
    VulkanDescriptorPool* m_pVulkanDescriptorPool;
    std::vector<VulkanDescriptorSet*> m_pVulkanDescriptorSets;
    Renderer* m_pRenderer;
    Model* m_pVikingModel;
    Texture* m_pVikingTexture;

    VertexBuffer* m_pVertexBuffer;
    IndexBuffer* m_pIndexBuffer;
    std::vector<UniformBuffer*> m_pUniformBuffers;
};