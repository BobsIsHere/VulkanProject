#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanDescriptorPool;
class GraphicsPipeline;
class UniformBuffer;

class VulkanDescriptorSet
{
public:
	VulkanDescriptorSet(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool);
	~VulkanDescriptorSet();

	void Create(GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer, VkImageView textureImageView, VkSampler textureSampler);

	VkDescriptorSet GetDescriptorSet() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanDescriptorPool* m_pVulkanDescriptorPool;

	VkDescriptorSet m_DescriptorSet;
};