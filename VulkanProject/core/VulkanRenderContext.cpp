#include <array>
#include <iostream>

#include "VulkanRenderContext.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanImage.h"

VulkanRenderContext::VulkanRenderContext(VulkanDevice* pDevice, VulkanSwapChain* pSwapChain) :
	m_pDevice{ pDevice },
	m_pSwapChain{ pSwapChain },
	m_ColorFormat{ pSwapChain->GetSwapChainImageFormat() },
	m_DepthFormat{  }
{
}

VulkanRenderContext::~VulkanRenderContext()
{
}

void VulkanRenderContext::BuildRenderingInfo(uint32_t imageIdx, VulkanImage* pDepthImage, VkRenderingInfo& outRenderInfo,
    VkRenderingAttachmentInfo& outColorAttachment, VkRenderingAttachmentInfo& outDepthAttachment) const
{
    // Color attachment
    outColorAttachment = {};
    outColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    outColorAttachment.imageView = m_pSwapChain->GetSwapChainImageViews()[imageIdx];
    outColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    outColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    outColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    outColorAttachment.clearValue = { {0.0f, 0.0f, 0.0f, 1.0f} };

    // Depth attachment
    outDepthAttachment = {};
    outDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    outDepthAttachment.imageView = pDepthImage->GetImageView();

    if (HasStencilComponent(pDepthImage->GetFormat()))
    {
        outDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    else
    {
        outDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    }

    outDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    outDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    outDepthAttachment.clearValue = { {1.0f, 0} };

    // Rendering info
    outRenderInfo = {};
    outRenderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    outRenderInfo.renderArea.offset = { 0, 0 };
    outRenderInfo.renderArea.extent = m_pSwapChain->GetSwapChainExtent();
    outRenderInfo.layerCount = 1;
    outRenderInfo.colorAttachmentCount = 1;
    outRenderInfo.pColorAttachments = &outColorAttachment;
    outRenderInfo.pDepthAttachment = &outDepthAttachment;
    outRenderInfo.pStencilAttachment = nullptr;
}

VkFormat VulkanRenderContext::GetColorFormat() const
{
	return m_ColorFormat;
}

VkFormat VulkanRenderContext::GetDepthFormat() const
{
	return m_DepthFormat;
}

bool VulkanRenderContext::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        format == VK_FORMAT_D24_UNORM_S8_UINT;
}
