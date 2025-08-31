#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanRenderContext;

class GraphicsPipeline final
{
public:
	GraphicsPipeline(VulkanDevice* pDevice, VulkanRenderContext* pRenderPass);
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
	VulkanRenderContext* m_pVulkanRenderPass;

	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	VkDescriptorSetLayout m_DescriptorSetLayout;

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
};