#include <vector>
#include <iostream>

#include "VulkanDescriptorSet.h"
#include "core/VulkanDevice.h"
#include "core/VulkanDescriptorPool.h"
#include "pipelines/GraphicsPipeline.h"
#include "buffers/UniformBuffer.h"
#include "buffers/VertexBuffer.h"
#include "utils/utils.h"

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool) :
	m_pVulkanDevice{ pDevice },
	m_pVulkanDescriptorPool{ pDescriptorPool },
	m_DescriptorSet{}
{
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
}

void VulkanDescriptorSet::Create(GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer, VertexBuffer* pVertexBuffer, VkImageView textureImageView, VkSampler textureSampler)
{
    VkDescriptorSetLayout layout{ pPipeline->GetDescriptorSetLayout() };

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pVulkanDescriptorPool->GetDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(m_pVulkanDevice->GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = pUniformBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    // Write SSBO handle to descriptor set
    VkDescriptorBufferInfo vertexBufferInfo{};
    vertexBufferInfo.buffer = pVertexBuffer->GetBuffer();
    vertexBufferInfo.offset = 0;
    vertexBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_DescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_DescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &vertexBufferInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_DescriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_pVulkanDevice->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkDescriptorSet VulkanDescriptorSet::GetDescriptorSet() const
{
	return m_DescriptorSet;
}
