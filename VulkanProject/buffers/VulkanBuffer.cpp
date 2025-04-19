#include <iostream>

#include "VulkanBuffer.h"
#include "core/VulkanDevice.h"
#include "core/VulkanCommandBuffer.h"

VulkanBuffer::VulkanBuffer(VulkanDevice* pDevice) :
	m_pVulkanDevice{ pDevice },
	m_Buffer{},
	m_BufferMemory{}
{
}

VulkanBuffer::~VulkanBuffer()
{
}

void VulkanBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_pVulkanDevice->GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_pVulkanDevice->GetDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_pVulkanDevice->GetDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_pVulkanDevice->GetDevice(), buffer, bufferMemory, 0);
}

void VulkanBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VulkanCommandBuffer commandBuffer{};

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void VulkanBuffer::Cleanup()
{
    vkDestroyBuffer(m_pVulkanDevice->GetDevice(), m_Buffer, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), m_BufferMemory, nullptr);
}

uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_pVulkanDevice->GetPhysicalDevice(), &memProperties);

    for (uint32_t idx = 0; idx < memProperties.memoryTypeCount; ++idx)
    {
        if ((typeFilter & (1 << idx)) && (memProperties.memoryTypes[idx].propertyFlags & properties) == properties)
        {
            return idx;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
