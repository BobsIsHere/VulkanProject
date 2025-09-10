#define STB_IMAGE_IMPLEMENTATION
#include <3rdParty/stb_image.h>

#include <iostream>

#include "Texture.h"
#include "VulkanDevice.h"
#include "utils/CommandUtils.h"

Texture::Texture(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, std::string fileName) :
	VulkanImage(pDevice),
	m_pVulkanDevice{ pDevice },
    m_pVulkanCommandPool{ pCommandPool },
	m_Sampler{},
    m_FileName{ fileName },
    m_Type{}
{
}

Texture::Texture(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool, const aiTexture* embeddedTexture) :
    VulkanImage(pDevice),
    m_pVulkanDevice{ pDevice },
    m_pVulkanCommandPool{ pCommandPool },
    m_Sampler{},
    m_FileName{},
	m_Type{ TextureType::Unknown }
{
    if (embeddedTexture->mHeight == 0)
    {
        CreateTextureImageFromMemory(
            reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
            embeddedTexture->mWidth);
    }
    else 
    {
        // Raw pixel data
        int texWidth = embeddedTexture->mWidth;
        int texHeight = embeddedTexture->mHeight;
        const unsigned char* pixels = reinterpret_cast<const unsigned char*>(embeddedTexture->pcData);
        
		CreateTextureImageFromMemory(pixels, texWidth * texHeight * 4);
    }
}

Texture::~Texture()
{
    Cleanup();
}

void Texture::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels{ stbi_load(m_FileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
    VkDeviceSize imageSize{ static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4 };

    if (!pixels)
    {
        std::cerr << "stbi_load failed: " << stbi_failure_reason() << std::endl;
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

    TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_pVulkanCommandPool);
    CopyBufferToImage(stagingBuffer, m_Image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_pVulkanCommandPool);

    vkDestroyBuffer(m_pVulkanDevice->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, nullptr);
}

void Texture::CreateTextureImageFromMemory(const unsigned char* pixels, size_t size)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* decodedPixels{ stbi_load_from_memory(pixels, static_cast<int>(size), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
    VkDeviceSize imageSize{ static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4 };

    if (!decodedPixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, decodedPixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory);

    stbi_image_free(decodedPixels);

    CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

    TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_pVulkanCommandPool);
    CopyBufferToImage(stagingBuffer, m_Image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_pVulkanCommandPool);

    vkDestroyBuffer(m_pVulkanDevice->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, nullptr);
}

void Texture::CreateTextureImageView()
{
    m_ImageView = CreateImageView(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_pVulkanDevice->GetPhysicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;

    if (vkCreateSampler(m_pVulkanDevice->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Texture::Cleanup()
{
    vkDestroySampler(m_pVulkanDevice->GetDevice(), m_Sampler, nullptr);
	VulkanImage::Cleanup();
}

VkSampler Texture::GetSampler() const
{
    return m_Sampler;
}

std::string Texture::GetFileName() const
{
    return m_FileName;
}

void Texture::SetType(TextureType type)
{
	m_Type = type;
}

TextureType Texture::GetType() const
{
    return m_Type;
}

void Texture::SetBindlessIndex(uint32_t index)
{
	m_BindlessIndex = index;
}

uint32_t Texture::GetBindlessIndex() const
{
    return m_BindlessIndex;
}

void Texture::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_pVulkanDevice->GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_pVulkanDevice->GetDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_pVulkanDevice->GetDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_pVulkanDevice->GetDevice(), buffer, bufferMemory, 0);
}

void Texture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = CommandUtils::BeginSingleTimeCommands(m_pVulkanDevice, m_pVulkanCommandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    CommandUtils::EndSingleTimeCommands(m_pVulkanDevice, m_pVulkanCommandPool, commandBuffer);
}

uint32_t Texture::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_pVulkanDevice->GetPhysicalDevice(), &memProperties);

    for (uint32_t idx = 0; idx < memProperties.memoryTypeCount; ++idx)
    {
        if ((typeFilter & (1 << idx)) && (memProperties.memoryTypes[idx].propertyFlags & properties) == properties)
        {
            return idx;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
