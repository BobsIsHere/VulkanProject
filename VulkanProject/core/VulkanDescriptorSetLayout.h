#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>

class VulkanDevice;

class VulkanDescriptorSetLayout final
{
public:
	VulkanDescriptorSetLayout(VulkanDevice* pDevice);
	~VulkanDescriptorSetLayout();

	void Create();
	void Cleanup();

	VkDescriptorSetLayout GetGlobalSetLayout() const;
	VkDescriptorSetLayout GetUBOSetLayout() const;

private:
	VulkanDevice* m_pDevice;

	VkDescriptorSetLayout m_GlobalDataSetLayout;
	VkDescriptorSetLayout m_UBOSetLayout;

	std::vector<VkDescriptorBindingFlags> m_BindingFlags;
};