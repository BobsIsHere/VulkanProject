#include <iostream>

#include "VulkanCommandBuffer.h"
#include "utils/utils.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "pipelines/GraphicsPipeline.h"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, VulkanRenderPass* pRenderPass, VulkanSwapChain* pSwapChain, 
    GraphicsPipeline* pGraphicsPipeline) :
	m_pVulkanDevice{ pDevice },
	m_pVulkanCommandPool{ pCommandPool },
	m_pVulkanRenderPass{ pRenderPass },
	m_pVulkanSwapChain{ pSwapChain },
	m_pGraphicsPipeline{ pGraphicsPipeline },
	m_CommandBuffer{}
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

void VulkanCommandBuffer::Create()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_pVulkanCommandPool->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_pVulkanDevice->GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanCommandBuffer::Record(uint32_t imageIdx)
{
    //VkCommandBufferBeginInfo beginInfo{};
    //beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //beginInfo.flags = 0; // Optional
    //beginInfo.pInheritanceInfo = nullptr; // Optional

    //if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS)
    //{
    //    throw std::runtime_error("failed to begin recording command buffer!");
    //}

    //VkRenderPassBeginInfo renderPassInfo{};
    //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //renderPassInfo.renderPass = m_pVulkanRenderPass->GetRenderPass();
    //renderPassInfo.framebuffer = swapChainFramebuffers[imageIdx];
    //renderPassInfo.renderArea.offset = { 0, 0 };
    //renderPassInfo.renderArea.extent = m_pVulkanSwapChain->GetSwapChainExtent();

    //std::array<VkClearValue, 2> clearValues{};
    //clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    //clearValues[1].depthStencil = { 1.0f, 0 };

    //renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    //renderPassInfo.pClearValues = clearValues.data();

    //vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetGraphicsPipeline());

    //VkViewport viewport{};
    //viewport.x = 0.0f;
    //viewport.y = 0.0f;
    //viewport.width = static_cast<float>(m_pVulkanSwapChain->GetSwapChainExtent().width);
    //viewport.height = static_cast<float>(m_pVulkanSwapChain->GetSwapChainExtent().height);
    //viewport.minDepth = 0.0f;
    //viewport.maxDepth = 1.0f;
    //vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

    //VkRect2D scissor{};
    //scissor.offset = { 0, 0 };
    //scissor.extent = m_pVulkanSwapChain->GetSwapChainExtent();
    //vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);

    //VkBuffer vertexBuffers[] = { vertexBuffer };
    //VkDeviceSize offsets[] = { 0 };
    //vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, vertexBuffers, offsets);

    //vkCmdBindIndexBuffer(m_CommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    //vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetPipelineLayout(), 0, 1,
    //    &descriptorSets[currentFrame], 0, nullptr);
    //vkCmdDrawIndexed(m_CommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    //vkCmdEndRenderPass(m_CommandBuffer);

    //if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
    //{
    //    throw std::runtime_error("failed to record command buffer!");
    //}
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer() const
{
    return m_CommandBuffer;
}
