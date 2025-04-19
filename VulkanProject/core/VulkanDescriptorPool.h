#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;

class VulkanDescriptorPool final
{
public:
	VulkanDescriptorPool(VulkanDevice* pDevice);
	~VulkanDescriptorPool();

	void Create();
	void Cleanup();

	VkDescriptorPool GetDescriptorPool() const;

private:
	VulkanDevice* m_pVulkanDevice;

	VkDescriptorPool m_DescriptorPool;
};