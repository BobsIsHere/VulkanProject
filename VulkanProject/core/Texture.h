#pragma once

#include <string>

#include "VulkanImage.h"

class VulkanDevice;
class VulkanCommandPool;

class Texture final : public VulkanImage
{
public:
	Texture(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, std::string fileName);
	~Texture();

	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();
	void CleanupSampler();

	VkSampler GetSampler() const;

private:
	VulkanCommandPool* m_pVulkanCommandPool;

	VkSampler m_Sampler;
	std::string m_FileName;

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};