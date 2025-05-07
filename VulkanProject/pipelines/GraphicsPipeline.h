#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanRenderPass;

class GraphicsPipeline final
{
public:
	GraphicsPipeline(VulkanDevice* pDevice, VulkanRenderPass* pRenderPass);
	~GraphicsPipeline();

	void CreatePipeline();
	void CreateDescriptorSetLayout();

	void CleanupPipeline();
	void CleanupDescriptorSetLayout();

	VkPipelineLayout GetPipelineLayout() const;
	VkPipeline GetGraphicsPipeline() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;

private:
	VulkanDevice* m_pVulkanDevice;
	VulkanRenderPass* m_pVulkanRenderPass;

	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	VkDescriptorSetLayout m_DescriptorSetLayout;

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
};