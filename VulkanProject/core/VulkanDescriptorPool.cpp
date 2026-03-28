#include <array>
#include <iostream>

#include "VulkanDescriptorPool.h"
#include "VulkanDevice.h"
#include "utils/utils.h"

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* pDevice) :
	m_pVulkanDevice{ pDevice },
	m_DescriptorPool{}
{
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
}

void VulkanDescriptorPool::Create(int numModels)
{
    const uint32_t totalSets{ static_cast<uint32_t>(numModels * utils::MAX_FRAMES_IN_FLIGHT) };

    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = totalSets;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[1].descriptorCount = totalSets;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[2].descriptorCount = totalSets * utils::TEXTURE_ARRAY_SIZE;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    // vertex buffer + material buffer
    poolSizes[3].descriptorCount = totalSets * 2;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = totalSets;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    if (vkCreateDescriptorPool(m_pVulkanDevice->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanDescriptorPool::Cleanup()
{
    vkDestroyDescriptorPool(m_pVulkanDevice->GetDevice(), m_DescriptorPool, nullptr);
}

VkDescriptorPool VulkanDescriptorPool::GetDescriptorPool() const
{
    return m_DescriptorPool;
}
