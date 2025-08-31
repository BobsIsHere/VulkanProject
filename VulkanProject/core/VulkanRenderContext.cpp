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
    outDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
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

//void VulkanRenderContext::Create()
//{
//    VkAttachmentDescription2 colorAttachment{};
//    colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
//	colorAttachment.pNext = nullptr;
//    colorAttachment.format = m_pSwapChain->GetSwapChainImageFormat();
//    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//    VkAttachmentReference2 colorAttachmentRef{};
//	colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
//    colorAttachmentRef.attachment = 0;
//    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentDescription2 depthAttachment{};
//	depthAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
//    depthAttachment.format = FindDepthFormat();
//    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentReference2 depthAttachmentRef{};
//	depthAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
//    depthAttachmentRef.attachment = 1;
//    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//    VkSubpassDescription2 subpass{};
//	subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
//    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    subpass.colorAttachmentCount = 1;
//    subpass.pColorAttachments = &colorAttachmentRef;
//    subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//    VkSubpassDependency2 dependency{};
//	dependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
//    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//    dependency.dstSubpass = 0;
//    dependency.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
//    dependency.srcAccessMask = 0;
//    dependency.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
//    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//    std::array<VkAttachmentDescription2, 2> attachments = { colorAttachment, depthAttachment };
//    VkRenderPassCreateInfo2 renderPassInfo{};
//    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
//    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//    renderPassInfo.pAttachments = attachments.data();
//    renderPassInfo.subpassCount = 1;
//    renderPassInfo.pSubpasses = &subpass;
//    renderPassInfo.dependencyCount = 1;
//    renderPassInfo.pDependencies = &dependency;
//
//    if (vkCreateRenderPass2(m_pDevice->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to create render pass!");
//    }
//}
//
//void VulkanRenderContext::Cleanup()
//{
//    vkDestroyRenderPass(m_pDevice->GetDevice(), m_RenderPass, nullptr);
//}
//
//VkRenderPass VulkanRenderContext::GetRenderPass() const
//{
//	return m_RenderPass;
//}
//
//VkFormat VulkanRenderContext::FindDepthFormat()
//{
//    return FindSupportedFormat(
//        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
//        VK_IMAGE_TILING_OPTIMAL,
//        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
//    );
//}
//
//VkFormat VulkanRenderContext::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
//{
//    for (VkFormat format : candidates)
//    {
//        VkFormatProperties props;
//        vkGetPhysicalDeviceFormatProperties(m_pDevice->GetPhysicalDevice(), format, &props);
//
//        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
//        {
//            return format;
//        }
//        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
//        {
//            return format;
//        }
//    }
//
//    throw std::runtime_error("failed to find supported format!");
//}
