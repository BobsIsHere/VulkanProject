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

    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();

    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void LoadModel();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    bool HasStencilComponent(VkFormat format);

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

    VertexBuffer* m_pVertexBuffer;
    IndexBuffer* m_pIndexBuffer;
    std::vector<UniformBuffer*> m_pUniformBuffers;

    // Vulkan Member Variables
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView textureImageView;
    VkSampler textureSampler;
};