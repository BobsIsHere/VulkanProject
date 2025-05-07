#include <iostream>

#include "Renderer.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImage.h"
#include "buffers/UniformBuffer.h"
#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "Window.h"

Renderer::Renderer(VulkanDevice* pDevice, VulkanSwapChain* pSwapChain, VulkanRenderPass* pRenderPass, Window* pWindow) :
    m_pVulkanDevice{ pDevice },
    m_pVulkanSwapChain{ pSwapChain },
    m_pVulkanRenderPass{ pRenderPass },
    m_pWindow{ pWindow },
    m_pDepthImage{ new VulkanImage(pDevice) }
{
}

Renderer::~Renderer()
{
}

void Renderer::CreateDepthResources()
{
    m_pDepthImage->Create(m_pVulkanSwapChain);
}

void Renderer::Cleanup()
{
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        vkDestroySemaphore(m_pVulkanDevice->GetDevice(), m_RenderFinishedSemaphores[idx], nullptr);
        vkDestroySemaphore(m_pVulkanDevice->GetDevice(), m_ImageAvailableSemaphores[idx], nullptr);
        vkDestroyFence(m_pVulkanDevice->GetDevice(), m_InFlightFences[idx], nullptr);
    }
}

void Renderer::CreateFrameBuffers()
{
    m_SwapChainFrameBuffers.resize(m_pVulkanSwapChain->GetSwapChainImageViews().size());
    auto temp = m_pVulkanSwapChain->GetSwapChainImageViews().size();

    for (size_t idx = 0; idx < m_pVulkanSwapChain->GetSwapChainImageViews().size(); ++idx)
    {
        std::array<VkImageView, 2> attachments = {
            m_pVulkanSwapChain->GetSwapChainImageViews()[idx],
            m_pDepthImage->GetImageView()
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
            &m_SwapChainFrameBuffers[idx]) != VK_SUCCESS)
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
    m_pDepthImage->Cleanup();

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
    m_pDepthImage->Create(m_pVulkanSwapChain);
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

    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        if (vkCreateSemaphore(m_pVulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[idx]) != VK_SUCCESS ||
            vkCreateSemaphore(m_pVulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[idx]) != VK_SUCCESS ||
            vkCreateFence(m_pVulkanDevice->GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}