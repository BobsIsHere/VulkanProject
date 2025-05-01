#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice;
class VulkanCommandPool;
class VulkanSwapChain;

class VulkanImage final
{
public:
	VulkanImage(VulkanDevice* pDevice);
	~VulkanImage();

	void Create(VulkanSwapChain* pSwapChain);
	void Cleanup();

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VulkanCommandPool* pCommandPool);

	VkImage GetImage() const;
	VkDeviceMemory GetImageMemory() const;
	VkImageView GetImageView() const;
	VkFormat GetFormat() const;

private:
	VulkanDevice* m_pVulkanDevice;

	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;
	VkFormat m_Format;

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	VkFormat FindFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};