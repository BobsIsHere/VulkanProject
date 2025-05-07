#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;

class VulkanCommandPool final
{
public:
	VulkanCommandPool(VulkanDevice* pDevice);
	~VulkanCommandPool();

	void Create();
	void Cleanup();

	VkCommandPool GetCommandPool() const;

private:
	VulkanDevice* m_pVulkanDevice;

	VkCommandPool m_CommandPool;
};