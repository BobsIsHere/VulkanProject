#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanDevice* pDevice);
	~VulkanBuffer();

	virtual void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	virtual void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	virtual void Cleanup();

private:
	VulkanDevice* m_pVulkanDevice;

	VkBuffer m_Buffer;
	VkDeviceMemory m_BufferMemory;

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};