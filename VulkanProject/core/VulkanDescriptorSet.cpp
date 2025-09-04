#include <vector>
#include <iostream>

#include "VulkanDescriptorSet.h"
#include "core/VulkanDevice.h"
#include "core/VulkanDescriptorPool.h"
#include "pipelines/GraphicsPipeline.h"
#include "buffers/UniformBuffer.h"
#include "utils/utils.h"

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool) :
	m_pVulkanDevice{ pDevice },
	m_pVulkanDescriptorPool{ pDescriptorPool },
	m_GlobalDescriptorSet{},
    m_FrameDescriptorSet{}
{
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
}

void VulkanDescriptorSet::Create(GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer, VkImageView textureImageView, VkSampler textureSampler)
{
    // ---- Allocate descriptor set for UBO (set = 0) ----
    VkDescriptorSetLayout uboLayout{ pPipeline->GetGlobalSetLayout() };
    VkDescriptorSetAllocateInfo uboAllocInfo{};
    uboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    uboAllocInfo.descriptorPool = m_pVulkanDescriptorPool->GetDescriptorPool();
    uboAllocInfo.descriptorSetCount = 1;
    uboAllocInfo.pSetLayouts = &uboLayout;

    if (vkAllocateDescriptorSets(m_pVulkanDevice->GetDevice(), &uboAllocInfo, &m_GlobalDescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate UBO descriptor set!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = pUniformBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet uboWrite{};
    uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uboWrite.dstSet = m_GlobalDescriptorSet;
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_pVulkanDevice->GetDevice(), 1, &uboWrite, 0, nullptr);

    // ---- Allocate descriptor set for textures (set = 1) ----
    VkDescriptorSetLayout textureLayout{ pPipeline->GetFrameSetLayout() };
    VkDescriptorSetAllocateInfo texAllocInfo{};
    texAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    texAllocInfo.descriptorPool = m_pVulkanDescriptorPool->GetDescriptorPool();
    texAllocInfo.descriptorSetCount = 1;
    texAllocInfo.pSetLayouts = &textureLayout;

    if (vkAllocateDescriptorSets(m_pVulkanDevice->GetDevice(), &texAllocInfo, &m_FrameDescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate texture descriptor set!");
    }

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = textureSampler;

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstSet = m_FrameDescriptorSet;
    samplerWrite.dstBinding = 0;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerWrite.descriptorCount = 1;
    samplerWrite.pImageInfo = &samplerInfo;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = textureImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet imageWrite{};
    imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imageWrite.dstSet = m_FrameDescriptorSet;
    imageWrite.dstBinding = 1;
    imageWrite.dstArrayElement = 0;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    imageWrite.descriptorCount = 1;
    imageWrite.pImageInfo = &imageInfo;

    const std::array<VkWriteDescriptorSet, 2> writes = { samplerWrite, imageWrite };

    vkUpdateDescriptorSets(m_pVulkanDevice->GetDevice(),static_cast<uint32_t>(writes.size()),
        writes.data(),0, nullptr);
}

VkDescriptorSet VulkanDescriptorSet::GetGlobalDescriptorSet() const
{
	return m_GlobalDescriptorSet;
}

VkDescriptorSet VulkanDescriptorSet::GetFrameDescriptorSet() const
{
    return m_FrameDescriptorSet;
}
