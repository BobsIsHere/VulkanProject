#include <vector>
#include <iostream>

#include "VulkanDescriptorSet.h"
#include "core/VulkanDevice.h"
#include "core/VulkanDescriptorPool.h"
#include "core/Texture.h"
#include "core/VulkanDescriptorSetLayout.h"
#include "buffers/UniformBuffer.h"
#include "buffers/VertexBuffer.h"
#include "utils/utils.h"

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool) :
	m_pVulkanDevice{ pDevice },
	m_pVulkanDescriptorPool{ pDescriptorPool },
	m_UBODescriptorSet{},
    m_GlobalDescriptorSet{}
{
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
}

void VulkanDescriptorSet::Create(VulkanDescriptorSetLayout* pLayout, UniformBuffer* pUniformBuffer, VertexBuffer* pVertexBuffer, std::vector<Texture*> pTextures)
{
    // ---- Allocate descriptor set for UBO (set = 0) ----
    VkDescriptorSetLayout uboLayout{ pLayout->GetUBOSetLayout() };
    VkDescriptorSetAllocateInfo uboAllocInfo{};
    uboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    uboAllocInfo.descriptorPool = m_pVulkanDescriptorPool->GetDescriptorPool();
    uboAllocInfo.descriptorSetCount = 1;
    uboAllocInfo.pSetLayouts = &uboLayout;

    if (vkAllocateDescriptorSets(m_pVulkanDevice->GetDevice(), &uboAllocInfo, &m_UBODescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate UBO descriptor set!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = pUniformBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet uboWrite{};
    uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uboWrite.dstSet = m_UBODescriptorSet;
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

	VkDescriptorBufferInfo vertexBufferInfo{};
	vertexBufferInfo.buffer = pVertexBuffer->GetBuffer();
	vertexBufferInfo.offset = 0;
	vertexBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet vertexBufferWrite{};
	vertexBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vertexBufferWrite.dstSet = m_UBODescriptorSet;
	vertexBufferWrite.dstBinding = 1;
	vertexBufferWrite.dstArrayElement = 0;
	vertexBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexBufferWrite.descriptorCount = 1;
	vertexBufferWrite.pBufferInfo = &vertexBufferInfo;

    const std::array<VkWriteDescriptorSet, 2> uboWrites = { uboWrite, vertexBufferWrite };
    vkUpdateDescriptorSets(m_pVulkanDevice->GetDevice(), static_cast<uint32_t>(uboWrites.size()), uboWrites.data(), 0, nullptr);

    // ---- Allocate descriptor set for textures (set = 1) ----
    VkDescriptorSetLayout textureLayout{ pLayout->GetGlobalSetLayout() };
    VkDescriptorSetAllocateInfo texAllocInfo{};
    texAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    texAllocInfo.descriptorPool = m_pVulkanDescriptorPool->GetDescriptorPool();
    texAllocInfo.descriptorSetCount = 1;
    texAllocInfo.pSetLayouts = &textureLayout;

    if (vkAllocateDescriptorSets(m_pVulkanDevice->GetDevice(), &texAllocInfo, &m_GlobalDescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate texture descriptor set!");
    }

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = pTextures[0]->GetSampler();

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstSet = m_GlobalDescriptorSet;
    samplerWrite.dstBinding = 0;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerWrite.descriptorCount = 1;
    samplerWrite.pImageInfo = &samplerInfo;

    const uint32_t textureCount{ static_cast<uint32_t>(pTextures.size()) };
    std::vector<VkDescriptorImageInfo> imageInfos(textureCount);

    for (size_t textureIdx = 0; textureIdx < textureCount; ++textureIdx)
    {
        imageInfos[textureIdx].sampler = VK_NULL_HANDLE;
        imageInfos[textureIdx].imageView = pTextures[textureIdx]->GetImageView();
        imageInfos[textureIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

	// Fill remaining slots with a fallback white texture if fewer textures are provided
    //VkImageView fallbackView = GetFallbackWhiteTextureImageView();

    VkWriteDescriptorSet texturesWrite{};
    texturesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    texturesWrite.dstSet = m_GlobalDescriptorSet;
    texturesWrite.dstBinding = 1;
    texturesWrite.dstArrayElement = 0;
    texturesWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texturesWrite.descriptorCount = textureCount;
    texturesWrite.pImageInfo = imageInfos.data();

    const std::array<VkWriteDescriptorSet, 2> writes = { samplerWrite, texturesWrite };
    vkUpdateDescriptorSets(m_pVulkanDevice->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

VkDescriptorSet VulkanDescriptorSet::GetUBODescriptorSet() const
{
	return m_UBODescriptorSet;
}

VkDescriptorSet VulkanDescriptorSet::GetGlobalDescriptorSet() const
{
    return m_GlobalDescriptorSet;
}
