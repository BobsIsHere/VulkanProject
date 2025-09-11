#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>

class VulkanDevice;
class VulkanDescriptorPool;
class VulkanDescriptorSetLayout;
class UniformBuffer;
class VertexBuffer;
class Texture;

class VulkanDescriptorSet final
{
public:
	VulkanDescriptorSet(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool);
	~VulkanDescriptorSet();

	void Create(VulkanDescriptorSetLayout* pLayout, UniformBuffer* pUniformBuffer, VertexBuffer* pVertexBuffer, std::vector<Texture*> pTextures);

	VkDescriptorSet GetUBODescriptorSet() const;
	VkDescriptorSet GetGlobalDescriptorSet() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanDescriptorPool* m_pVulkanDescriptorPool;

	VkDescriptorSet m_UBODescriptorSet;
	VkDescriptorSet m_GlobalDescriptorSet;
};