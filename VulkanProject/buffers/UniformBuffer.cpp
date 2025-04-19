#include "UniformBuffer.h"
#include "core/VulkanDevice.h"
#include "core/VulkanCommandPool.h"

UniformBuffer::UniformBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool) :
	VulkanBuffer(pDevice, pCommandPool),
	m_pBufferMapped{}
{
}

UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::CreateUniformBuffer()
{
	VkDeviceSize bufferSize{ sizeof(UniformBufferObject) };

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffer, m_BufferMemory);

	vkMapMemory(m_pVulkanDevice->GetDevice(), m_BufferMemory, 0, bufferSize, 0, &m_pBufferMapped);
}