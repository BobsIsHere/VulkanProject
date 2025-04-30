#include <iostream>

#include "VulkanCommandBuffer.h"
#include "utils/utils.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "pipelines/GraphicsPipeline.h"

VulkanCommandBuffer::VulkanCommandBuffer() :
	m_CommandBuffer{}
{
    
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

void VulkanCommandBuffer::Create(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pCommandPool->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(pDevice->GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffer!");
    }
}

void VulkanCommandBuffer::Reset(VkCommandBufferResetFlags flags) const
{
    if (vkResetCommandBuffer(m_CommandBuffer, flags) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to reset command buffer!");
    }
}

void VulkanCommandBuffer::Begin(VkCommandBufferUsageFlags flags) const
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void VulkanCommandBuffer::End() const
{
    if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanCommandBuffer::Submit(VkQueue queue, const VkSubmitInfo& info, VkFence fence) const
{
    if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer!");
    }
}

void VulkanCommandBuffer::SetCommandBuffer(VkCommandBuffer commandBuffer)
{
    m_CommandBuffer = commandBuffer;
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer() const
{
    return m_CommandBuffer;
}

void VulkanCommandBuffer::Record(uint32_t imageIdx, std::vector<VkFramebuffer> swapChainFramebuffers, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, 
    VulkanRenderPass* pRenderPass, VulkanSwapChain* pSwapChain, GraphicsPipeline* pPipeline, std::vector<VulkanDescriptorSet*> pVulkanDescriptorSets,
    uint32_t currentFrame, std::vector<uint32_t> indices)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pRenderPass->GetRenderPass();
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIdx];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = pSwapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->GetGraphicsPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pSwapChain->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(pSwapChain->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = pSwapChain->GetSwapChainExtent();
    vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { pVertexBuffer->GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(m_CommandBuffer, pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

    const VkDescriptorSet descriptorSet{ pVulkanDescriptorSets[currentFrame]->GetDescriptorSet() };
    vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->GetPipelineLayout(), 0, 1,
        &descriptorSet, 0, nullptr);
    vkCmdDrawIndexed(m_CommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(m_CommandBuffer);

    if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}