#include <iostream>

#include "backends/imgui_impl_vulkan.h"

#include "VulkanCommandBuffer.h"
#include "utils/utils.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanRenderContext.h"
#include "VulkanSwapChain.h"
#include "VulkanImage.h"
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

void VulkanCommandBuffer::Record(uint32_t imageIdx, VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer, 
    VulkanRenderContext* pRenderContext, VulkanSwapChain* pSwapChain, GraphicsPipeline* pPipeline, std::vector<std::unique_ptr<VulkanDescriptorSet>>& pVulkanDescriptorSets,
    uint32_t currentFrame, std::vector<uint32_t> indices, ImDrawData* drawData, VulkanImage* pDepthImage, VulkanDevice* pDevice)
{
    VkRenderingInfo renderInfo{};
    VkRenderingAttachmentInfo colorAttachment{};
    VkRenderingAttachmentInfo depthAttachment{};

	pRenderContext->BuildRenderingInfo(imageIdx, pDepthImage, renderInfo, colorAttachment, depthAttachment);

    vkCmdBeginRendering(m_CommandBuffer, &renderInfo);

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

    vkCmdBindIndexBuffer(m_CommandBuffer, 
        pIndexBuffer->GetBuffer(), 
        0, 
        VK_INDEX_TYPE_UINT32);

    const VkDescriptorSet descriptorSet{ pVulkanDescriptorSets[currentFrame]->GetDescriptorSet() };
    vkCmdBindDescriptorSets(m_CommandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pPipeline->GetPipelineLayout(), 
        0, 
        1,
        &descriptorSet, 
        0, 
        nullptr);

    vkCmdDrawIndexed(m_CommandBuffer, 
        static_cast<uint32_t>(indices.size()), 
        1, 
        0, 
        0, 
        0);

	// Render ImGui
	if (drawData)
	{
		ImGui_ImplVulkan_RenderDrawData(drawData, m_CommandBuffer);
	}

    vkCmdEndRendering(m_CommandBuffer);
}