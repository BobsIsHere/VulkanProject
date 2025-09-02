#include <iostream>

#include "VertexBuffer.h"
#include "core/VulkanDevice.h"

VertexBuffer::VertexBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool) :
	VulkanBuffer(pDevice, pCommandPool)
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::CreateVertexBuffer(std::vector<Vertex> vertices)
{
    VkDeviceSize bufferSize{ sizeof(vertices[0]) * vertices.size() };

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, 
        stagingBufferMemory);

    void* data;
    vkMapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), size_t(bufferSize));
    vkUnmapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory);

    CreateBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        m_Buffer, 
        m_BufferMemory);

    CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

    vkDestroyBuffer(m_pVulkanDevice->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, nullptr);
}