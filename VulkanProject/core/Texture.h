#pragma once

#include <string>
#include <assimp/scene.h>

#include "VulkanImage.h"

class VulkanDevice;
class VulkanCommandPool;

enum class TextureType {
	Albedo,
	Normal,
	Metallic,
	Roughness,
	Unknown
};

class Texture final : public VulkanImage
{
public:
	Texture(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, std::string fileName);
	Texture(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, const aiTexture* embeddedTexture);
	~Texture();

	void CreateTextureImage();
	void CreateTextureImageFromMemory(const unsigned char* pixels, size_t size);
	void CreateTextureImageView();
	void CreateTextureSampler();
	void CleanupSampler();

	VkSampler GetSampler() const;
	std::string GetFileName() const;

	void SetType(TextureType type);
	TextureType GetType() const;

	void SetBindlessIndex(uint32_t index);
	uint32_t GetBindlessIndex() const;

private:
	VulkanCommandPool* m_pVulkanCommandPool;

	VkSampler m_Sampler;
	std::string m_FileName;

	TextureType m_Type;

	uint32_t m_BindlessIndex = UINT32_MAX;

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};