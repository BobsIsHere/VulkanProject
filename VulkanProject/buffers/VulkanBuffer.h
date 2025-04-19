#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanCommandPool;

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);
	~VulkanBuffer();

	virtual void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	virtual void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	virtual void Cleanup();

	VkBuffer GetBuffer() const;
	VkDeviceMemory GetBufferMemory() const;

protected:
	VulkanDevice* m_pVulkanDevice;
	VulkanCommandPool* m_pVulkanCommandPool;

	VkBuffer m_Buffer;
	VkDeviceMemory m_BufferMemory;

private:
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};