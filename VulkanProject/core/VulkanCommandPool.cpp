#include <iostream>

#include "VulkanCommandPool.h"
#include "utils.h"
#include "VulkanDevice.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice* pDevice) :
	m_pVulkanDevice{ pDevice },
	m_CommandPool{}
{
}

VulkanCommandPool::~VulkanCommandPool()
{
}

void VulkanCommandPool::Create()
{
    QueueFamilyIndices queueFamilyIndices = m_pVulkanDevice->FindQueueFamilies(m_pVulkanDevice->GetPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_pVulkanDevice->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanCommandPool::Cleanup()
{
    vkDestroyCommandPool(m_pVulkanDevice->GetDevice(), m_CommandPool, nullptr);
}

VkCommandPool VulkanCommandPool::GetCommandPool() const
{
    return m_CommandPool;
}
