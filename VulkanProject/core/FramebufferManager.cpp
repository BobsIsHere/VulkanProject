#include <iostream>

#include "FramebufferManager.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderContext.h"
#include "VulkanImage.h"

void FramebufferManager::CreateFramebuffers(VulkanDevice* pDevice, VulkanSwapChain* pSwapChain, VulkanRenderContext* pRenderpass, VulkanImage* pDepthImage)
{
    m_Framebuffers.resize(pSwapChain->GetSwapChainImageViews().size());
    auto temp = pSwapChain->GetSwapChainImageViews().size();

    for (size_t idx = 0; idx < pSwapChain->GetSwapChainImageViews().size(); ++idx)
    {
        std::array<VkImageView, 2> attachments = {
            pSwapChain->GetSwapChainImageViews()[idx],
            pDepthImage->GetImageView()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = nullptr;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = pSwapChain->GetSwapChainExtent().width;
        framebufferInfo.height = pSwapChain->GetSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(pDevice->GetDevice(), &framebufferInfo, nullptr,&m_Framebuffers[idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void FramebufferManager::Cleanup(VulkanDevice* pDevice)
{
    for (size_t idx = 0; idx < m_Framebuffers.size(); ++idx)
    {
        vkDestroyFramebuffer(pDevice->GetDevice(), m_Framebuffers[idx], nullptr);
    }
}

const std::vector<VkFramebuffer>& FramebufferManager::GetFramebuffers() const
{
	return m_Framebuffers;
}
