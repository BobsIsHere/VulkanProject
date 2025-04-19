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

#define STB_IMAGE_IMPLEMENTATION
#include <3rdParty/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <3rdParty/tiny_obj_loader.h>

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

#include "pipelines/GraphicsPipeline.h"

#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "buffers/UniformBuffer.h"

class HelloTriangleApplication 
{
public:
    void run() 
    {
        // Make Window
		m_pWindow = new Window();
		m_pWindow->Initialize(utils::WINDOW_WIDTH, utils::WINDOW_HEIGHT, "Vulkan Window");

		// Initialize Vulkan
		m_pVulkanInstance = new VulkanInstance();
		m_pVulkanDevice = new VulkanDevice(m_pVulkanInstance, m_pWindow);
        m_pVulkanSwapChain = new VulkanSwapChain(m_pWindow, m_pVulkanDevice);
		m_pVulkanRenderPass = new VulkanRenderPass(m_pVulkanDevice, m_pVulkanSwapChain);
		m_pVulkanCommandPool = new VulkanCommandPool(m_pVulkanDevice);
        m_pVulkanCommandBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);
        
        for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
        {
			m_pVulkanCommandBuffers[idx] = new VulkanCommandBuffer(m_pVulkanDevice, m_pVulkanCommandPool, m_pVulkanRenderPass, m_pVulkanSwapChain, m_pGraphicsPipeline);
        }

		m_pVulkanDescriptorPool = new VulkanDescriptorPool(m_pVulkanDevice);

		// Initialize Graphics Pipeline
		m_pGraphicsPipeline = new GraphicsPipeline(m_pVulkanDevice, m_pVulkanRenderPass);

		// Initialize Buffers
		m_pUniformBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);

		m_pVertexBuffer = new VertexBuffer(m_pVulkanDevice, m_pVulkanCommandPool);
		m_pIndexBuffer = new IndexBuffer(m_pVulkanDevice, m_pVulkanCommandPool);

        for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
        {
			m_pUniformBuffers[idx] = new UniformBuffer(m_pVulkanDevice, m_pVulkanCommandPool);
        }

        initVulkan();
        mainLoop();
        CleanupVulkan();
		CleanupResources();
    }

private:
    // Member Functions
    void initVulkan() 
    {
        m_pVulkanInstance->Initialize("Hello World!");
		m_pVulkanInstance->SetupDebugMessenger();

		m_pVulkanDevice->CreateSurface();
		m_pVulkanDevice->PickPhysicalDevice();
		m_pVulkanDevice->CreateLogicalDevice();

        m_pVulkanSwapChain->Create();

        m_pVulkanSwapChain->CreateImageViews();
        m_pVulkanRenderPass->Create();
        m_pGraphicsPipeline->CreateDescriptorSetLayout();
        m_pGraphicsPipeline->CreatePipeline();
        m_pVulkanCommandPool->Create();
        createDepthResources();
        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        loadModel();

        // Create Buffers
		m_pVertexBuffer->CreateVertexBuffer(vertices);
		m_pIndexBuffer->CreateIndexBuffer(indices);
		for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
		{
			m_pUniformBuffers[idx]->CreateUniformBuffer();
		}

		m_pVulkanDescriptorPool->Create();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop() 
    {
        while (!glfwWindowShouldClose(m_pWindow->GetWindow())) 
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(m_pVulkanDevice->GetDevice()); 
    }

    void CleanupVulkan() 
    {
        const auto& device{ m_pVulkanDevice->GetDevice() };

        cleanupSwapChain();

		m_pGraphicsPipeline->CleanupPipeline();

		m_pVulkanRenderPass->Cleanup();

        for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i) 
        {
			m_pUniformBuffers[i]->Cleanup();
        }

		m_pVulkanDescriptorPool->Cleanup();

        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);

        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);

		m_pGraphicsPipeline->CleanupDescriptorSetLayout();

		m_pIndexBuffer->Cleanup();
		m_pVertexBuffer->Cleanup();

        for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

		m_pVulkanCommandPool->Cleanup();

        m_pVulkanDevice->Cleanup();

		m_pVulkanInstance->Cleanup();

        glfwDestroyWindow(m_pWindow->GetWindow());

        glfwTerminate();
    }

    void CleanupResources()
    {
        delete m_pVulkanRenderPass;
        m_pVulkanRenderPass = nullptr;

        for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
        {
            delete m_pUniformBuffers[idx];
            m_pUniformBuffers[idx] = nullptr;
        }

		delete m_pVulkanDescriptorPool;
		m_pVulkanDescriptorPool = nullptr;

        delete m_pGraphicsPipeline;
        m_pGraphicsPipeline = nullptr;

        delete m_pIndexBuffer;
        m_pIndexBuffer = nullptr;

        delete m_pVertexBuffer;
		m_pVertexBuffer = nullptr;

		delete m_pVulkanCommandPool;
		m_pVulkanCommandPool = nullptr;

        delete m_pVulkanDevice;
        m_pVulkanDevice = nullptr;

        delete m_pVulkanInstance;
        m_pVulkanInstance = nullptr;
    }

    void createFramebuffers() 
    {
        swapChainFramebuffers.resize(m_pVulkanSwapChain->GetSwapChainImageViews().size());
        auto temp = m_pVulkanSwapChain->GetSwapChainImageViews().size();

        for (size_t i = 0; i < m_pVulkanSwapChain->GetSwapChainImageViews().size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
                m_pVulkanSwapChain->GetSwapChainImageViews()[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_pVulkanRenderPass->GetRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_pVulkanSwapChain->GetSwapChainExtent().width;
            framebufferInfo.height = m_pVulkanSwapChain->GetSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_pVulkanDevice->GetDevice(), &framebufferInfo, nullptr,
                &swapChainFramebuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createDepthResources() 
    {
        VkFormat depthFormat = findDepthFormat();

        createImage(m_pVulkanSwapChain->GetSwapChainExtent().width, m_pVulkanSwapChain->GetSwapChainExtent().height, depthFormat, 
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void createTextureImage() 
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(utils::TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) 
        {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_pVulkanDevice->GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, nullptr);
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) 
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_pVulkanDevice->GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_pVulkanDevice->GetDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_pVulkanDevice->GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_pVulkanDevice->GetDevice(), image, imageMemory, 0);
    }

    void createTextureImageView() 
    {
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_pVulkanDevice->GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void createTextureSampler() 
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_pVulkanDevice->GetPhysicalDevice(), &properties);

        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        if (vkCreateSampler(m_pVulkanDevice->GetDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void loadModel() 
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, utils::MODEL_PATH.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) 
        {
            for (const auto& index : shape.mesh.indices) 
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void createDescriptorSets() 
    {
        std::vector<VkDescriptorSetLayout> layouts(utils::MAX_FRAMES_IN_FLIGHT, m_pGraphicsPipeline->GetDescriptorSetLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pVulkanDescriptorPool->GetDescriptorPool();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(utils::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(utils::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_pVulkanDevice->GetDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_pUniformBuffers[i]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_pVulkanDevice->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createCommandBuffers() 
    {
        commandBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_pVulkanCommandPool->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(m_pVulkanDevice->GetDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_pVulkanRenderPass->GetRenderPass();
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_pVulkanSwapChain->GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetGraphicsPipeline());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_pVulkanSwapChain->GetSwapChainExtent().width);
            viewport.height = static_cast<float>(m_pVulkanSwapChain->GetSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = m_pVulkanSwapChain->GetSwapChainExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = { m_pVertexBuffer->GetBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetPipelineLayout(), 0, 1,
                &descriptorSets[currentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void drawFrame() 
    {
        vkWaitForFences(m_pVulkanDevice->GetDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_pVulkanDevice->GetDevice(), m_pVulkanSwapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_pUniformBuffers[currentFrame]->UpdateUniformBuffer(m_pVulkanSwapChain);

        vkResetFences(m_pVulkanDevice->GetDevice(), 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_pVulkanDevice->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_pVulkanSwapChain->GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_pVulkanDevice->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->GetFramebufferResized()) 
        {
            m_pWindow->SetFramebufferResized(false);
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % utils::MAX_FRAMES_IN_FLIGHT;
    }

    void createSyncObjects() 
    {
        imageAvailableSemaphores.resize(utils::MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(utils::MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(utils::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(m_pVulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_pVulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_pVulkanDevice->GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void cleanupSwapChain() 
    {
        vkDestroyImageView(m_pVulkanDevice->GetDevice(), depthImageView, nullptr);
        vkDestroyImage(m_pVulkanDevice->GetDevice(), depthImage, nullptr);
        vkFreeMemory(m_pVulkanDevice->GetDevice(), depthImageMemory, nullptr);

        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) 
        {
            vkDestroyFramebuffer(m_pVulkanDevice->GetDevice(), swapChainFramebuffers[i], nullptr);
        }

        for (size_t i = 0; i < m_pVulkanSwapChain->GetSwapChainImageViews().size(); i++)
        {
            vkDestroyImageView(m_pVulkanDevice->GetDevice(), m_pVulkanSwapChain->GetSwapChainImageViews()[i], nullptr);
        }

        m_pVulkanSwapChain->Cleanup();
    }

    void recreateSwapChain() 
    {
        int width = 0;
        int height = 0;

        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(m_pWindow->GetWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_pVulkanDevice->GetDevice());

        cleanupSwapChain();

        m_pVulkanSwapChain->Create();
		m_pVulkanSwapChain->CreateImageViews();
        createDepthResources();
        createFramebuffers();
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_pVulkanDevice->GetPhysicalDevice(), &memProperties);

        for (uint32_t idx = 0; idx < memProperties.memoryTypeCount; ++idx) 
        {
            if ((typeFilter & (1 << idx)) && (memProperties.memoryTypes[idx].propertyFlags & properties) == properties) 
            {
                return idx;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_pVulkanDevice->GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_pVulkanDevice->GetDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_pVulkanDevice->GetDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_pVulkanDevice->GetDevice(), buffer, bufferMemory, 0);
    }

    VkCommandBuffer beginSingleTimeCommands() 
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_pVulkanCommandPool->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_pVulkanDevice->GetDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) 
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_pVulkanDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_pVulkanDevice->GetGraphicsQueue());

        vkFreeCommandBuffers(m_pVulkanDevice->GetDevice(), m_pVulkanCommandPool->GetCommandPool(), 1, &commandBuffer);
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) 
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0; // TODO
        barrier.dstAccessMask = 0; // TODO

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else 
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer);
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
    {
        for (VkFormat format : candidates) 
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_pVulkanDevice->GetPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat() 
    {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool hasStencilComponent(VkFormat format) 
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

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

	VertexBuffer* m_pVertexBuffer;
	IndexBuffer* m_pIndexBuffer;
    std::vector<UniformBuffer*> m_pUniformBuffers;

    // Vulkan Member Variables
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0; 

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    std::vector<VkDescriptorSet> descriptorSets;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
};

int main()
{
    HelloTriangleApplication app;

    try 
    {
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
