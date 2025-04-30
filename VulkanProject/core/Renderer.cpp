#include <iostream>

#include "Renderer.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanCommandBuffer.h"
#include "buffers/UniformBuffer.h"
#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "Window.h"

Renderer::Renderer(VulkanDevice* device, VulkanSwapChain* swapChain, VulkanRenderPass* renderPass, Window* window) :
    m_pVulkanDevice{ device },
    m_pVulkanSwapChain{ swapChain },
    m_pVulkanRenderPass{ renderPass },
    m_pWindow{ window }
{
}

Renderer::~Renderer()
{
}

void Renderer::Cleanup()
{
    for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_pVulkanDevice->GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_pVulkanDevice->GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_pVulkanDevice->GetDevice(), m_InFlightFences[i], nullptr);
    }
}

void Renderer::CreateFrameBuffers()
{
    m_SwapChainFrameBuffers.resize(m_pVulkanSwapChain->GetSwapChainImageViews().size());
    auto temp = m_pVulkanSwapChain->GetSwapChainImageViews().size();

    for (size_t i = 0; i < m_pVulkanSwapChain->GetSwapChainImageViews().size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
            m_pVulkanSwapChain->GetSwapChainImageViews()[i],
            m_DepthImageView
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
            &m_SwapChainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Renderer::DrawFrame(std::vector<UniformBuffer*> pUniformBuffers, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, std::vector<VulkanCommandBuffer*> pCommandBuffers,
    GraphicsPipeline* pPipeline, std::vector<VulkanDescriptorSet*> pVulkanDescriptorSets, std::vector<uint32_t> indices)
{
    vkWaitForFences(m_pVulkanDevice->GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_pVulkanDevice->GetDevice(), m_pVulkanSwapChain->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], 
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    pUniformBuffers[m_CurrentFrame]->UpdateUniformBuffer(m_pVulkanSwapChain);

    vkResetFences(m_pVulkanDevice->GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    pCommandBuffers[m_CurrentFrame]->Reset();
    pCommandBuffers[m_CurrentFrame]->Begin();

    pCommandBuffers[m_CurrentFrame]->Record(imageIndex, m_SwapChainFrameBuffers, pVertexBuffer, pIndexBuffer, m_pVulkanRenderPass, 
        m_pVulkanSwapChain, pPipeline, pVulkanDescriptorSets, m_CurrentFrame, indices);

    pCommandBuffers[m_CurrentFrame]->End();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    const VkCommandBuffer commandBuffer{ pCommandBuffers[m_CurrentFrame]->GetCommandBuffer() };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_pVulkanDevice->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
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
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % utils::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::CleanupSwapChain()
{
    vkDestroyImageView(m_pVulkanDevice->GetDevice(), m_DepthImageView, nullptr);
    vkDestroyImage(m_pVulkanDevice->GetDevice(), m_DepthImage, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), m_DepthImageMemory, nullptr);

    for (size_t idx = 0; idx < m_SwapChainFrameBuffers.size(); ++idx)
    {
        vkDestroyFramebuffer(m_pVulkanDevice->GetDevice(), m_SwapChainFrameBuffers[idx], nullptr);
    }

    for (size_t idx = 0; idx < m_pVulkanSwapChain->GetSwapChainImageViews().size(); ++idx)
    {
        vkDestroyImageView(m_pVulkanDevice->GetDevice(), m_pVulkanSwapChain->GetSwapChainImageViews()[idx], nullptr);
    }

    m_pVulkanSwapChain->Cleanup();
}

void Renderer::RecreateSwapChain()
{
    int width = 0;
    int height = 0;

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_pWindow->GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_pVulkanDevice->GetDevice());

    CleanupSwapChain();

    m_pVulkanSwapChain->Create();
    m_pVulkanSwapChain->CreateImageViews();
    CreateDepthResources();
    CreateFrameBuffers();
}

void Renderer::CreateSyncObjects()
{
    m_ImageAvailableSemaphores.resize(utils::MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(utils::MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(utils::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(m_pVulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_pVulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_pVulkanDevice->GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Renderer::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();

    CreateImage(m_pVulkanSwapChain->GetSwapChainExtent().width, m_pVulkanSwapChain->GetSwapChainExtent().height, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_DepthImage, m_DepthImageMemory);
    m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkFormat Renderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

VkFormat Renderer::FindDepthFormat()
{
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void Renderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
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
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_pVulkanDevice->GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_pVulkanDevice->GetDevice(), image, imageMemory, 0);
}

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

VkImageView Renderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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