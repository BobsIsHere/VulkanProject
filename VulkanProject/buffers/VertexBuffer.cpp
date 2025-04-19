#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(VulkanDevice* pDevice) :
	VulkanBuffer(pDevice)
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::CreateVertexBuffer(std::vector<Vertex> vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(m_pVulkanDevice->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, nullptr);
}
