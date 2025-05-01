#define STB_IMAGE_IMPLEMENTATION
#include <3rdParty/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <3rdParty/tiny_obj_loader.h>

#include "VulkanProject.h"

void VulkanProject::Run()
{
    // Make Window
    m_pWindow = new Window();
    m_pWindow->Initialize(utils::WINDOW_WIDTH, utils::WINDOW_HEIGHT, "Vulkan Window");

    // Initialize Vulkan
    m_pVulkanInstance = new VulkanInstance();
    m_pVulkanDevice = new VulkanDevice(m_pVulkanInstance, m_pWindow);
    m_pVulkanSwapChain = new VulkanSwapChain(m_pWindow, m_pVulkanDevice);
    m_pVulkanRenderPass = new VulkanRenderPass(m_pVulkanDevice, m_pVulkanSwapChain);
    m_pVulkanCommandPool = new VulkanCommandPool(m_pVulkanDevice);
    m_pVulkanDescriptorPool = new VulkanDescriptorPool(m_pVulkanDevice);

    // Initialize Graphics Pipeline
    m_pGraphicsPipeline = new GraphicsPipeline(m_pVulkanDevice, m_pVulkanRenderPass);

    m_pVulkanCommandBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);
    m_pVulkanDescriptorSets.resize(utils::MAX_FRAMES_IN_FLIGHT);
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pVulkanCommandBuffers[idx] = new VulkanCommandBuffer();
        m_pVulkanDescriptorSets[idx] = new VulkanDescriptorSet(m_pVulkanDevice, m_pVulkanDescriptorPool);
    }

    // Initialize Buffers
    m_pUniformBuffers.resize(utils::MAX_FRAMES_IN_FLIGHT);

    m_pVertexBuffer = new VertexBuffer(m_pVulkanDevice, m_pVulkanCommandPool);
    m_pIndexBuffer = new IndexBuffer(m_pVulkanDevice, m_pVulkanCommandPool);

    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pUniformBuffers[idx] = new UniformBuffer(m_pVulkanDevice, m_pVulkanCommandPool);
    }

    m_pRenderer = new Renderer(m_pVulkanDevice, m_pVulkanSwapChain, m_pVulkanRenderPass, m_pWindow);

    InitVulkan();
    MainLoop();
    CleanupVulkan();
    CleanupResources();
}

void VulkanProject::InitVulkan()
{
    m_pVulkanInstance->Initialize("Hello World!");
    m_pVulkanInstance->SetupDebugMessenger();

    m_pVulkanDevice->CreateSurface();
    m_pVulkanDevice->PickPhysicalDevice();
    m_pVulkanDevice->CreateLogicalDevice();

    m_pVulkanSwapChain->Create();

    m_pVulkanSwapChain->CreateImageViews();
    m_pVulkanRenderPass->Create();
    m_pGraphicsPipeline->CreateDescriptorSetLayout();
    m_pGraphicsPipeline->CreatePipeline();
    m_pVulkanCommandPool->Create();
    m_pRenderer->CreateDepthResources();
    m_pRenderer->CreateFrameBuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    LoadModel();

    // Create Buffers
    m_pVertexBuffer->CreateVertexBuffer(vertices);
    m_pIndexBuffer->CreateIndexBuffer(indices);
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pUniformBuffers[idx]->CreateUniformBuffer();
    }

    m_pVulkanDescriptorPool->Create();
    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        m_pVulkanCommandBuffers[idx]->Create(m_pVulkanDevice, m_pVulkanCommandPool);
        m_pVulkanDescriptorSets[idx]->Create(m_pGraphicsPipeline, m_pUniformBuffers[idx], textureImageView, textureSampler);
    }

    m_pRenderer->CreateSyncObjects();
}

void VulkanProject::MainLoop()
{
    while (!glfwWindowShouldClose(m_pWindow->GetWindow()))
    {
        glfwPollEvents();
        m_pRenderer->DrawFrame(m_pUniformBuffers, m_pVertexBuffer, m_pIndexBuffer, m_pVulkanCommandBuffers, m_pGraphicsPipeline, m_pVulkanDescriptorSets,
            indices);
    }

    vkDeviceWaitIdle(m_pVulkanDevice->GetDevice());
}

void VulkanProject::CleanupVulkan()
{
    const auto& device{ m_pVulkanDevice->GetDevice() };

    m_pRenderer->CleanupSwapChain();

    m_pGraphicsPipeline->CleanupPipeline();

    m_pVulkanRenderPass->Cleanup();

    for (size_t i = 0; i < utils::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_pUniformBuffers[i]->Cleanup();
    }

    m_pVulkanDescriptorPool->Cleanup();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    m_pGraphicsPipeline->CleanupDescriptorSetLayout();

    m_pIndexBuffer->Cleanup();
    m_pVertexBuffer->Cleanup();

    m_pRenderer->Cleanup();

    m_pVulkanCommandPool->Cleanup();

    m_pVulkanDevice->Cleanup();

    m_pVulkanInstance->Cleanup();

    glfwDestroyWindow(m_pWindow->GetWindow());

    glfwTerminate();
}

void VulkanProject::CleanupResources()
{
    delete m_pRenderer;
    m_pRenderer = nullptr;

    delete m_pVulkanRenderPass;
    m_pVulkanRenderPass = nullptr;

    for (size_t idx = 0; idx < utils::MAX_FRAMES_IN_FLIGHT; ++idx)
    {
        delete m_pUniformBuffers[idx];
        m_pUniformBuffers[idx] = nullptr;

        delete m_pVulkanCommandBuffers[idx];
        m_pVulkanCommandBuffers[idx] = nullptr;
    }

    delete m_pVulkanDescriptorPool;
    m_pVulkanDescriptorPool = nullptr;

    delete m_pGraphicsPipeline;
    m_pGraphicsPipeline = nullptr;

    delete m_pIndexBuffer;
    m_pIndexBuffer = nullptr;

    delete m_pVertexBuffer;
    m_pVertexBuffer = nullptr;

    delete m_pVulkanCommandPool;
    m_pVulkanCommandPool = nullptr;

    delete m_pVulkanDevice;
    m_pVulkanDevice = nullptr;

    delete m_pVulkanInstance;
    m_pVulkanInstance = nullptr;
}

void VulkanProject::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(utils::TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
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
        VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_pVulkanDevice->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pVulkanDevice->GetDevice(), stagingBufferMemory, nullptr);
}

void VulkanProject::CreateTextureImageView()
{
    textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanProject::CreateTextureSampler()
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

    if (vkCreateSampler(m_pVulkanDevice->GetDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VulkanProject::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_pVulkanDevice->GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_pVulkanDevice->GetDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_pVulkanDevice->GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_pVulkanDevice->GetDevice(), image, imageMemory, 0);
}

VkImageView VulkanProject::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_pVulkanDevice->GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void VulkanProject::LoadModel()
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, utils::MODEL_PATH.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

uint32_t VulkanProject::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

void VulkanProject::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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

VkCommandBuffer VulkanProject::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_pVulkanCommandPool->GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_pVulkanDevice->GetDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanProject::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_pVulkanDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_pVulkanDevice->GetGraphicsQueue());

    vkFreeCommandBuffers(m_pVulkanDevice->GetDevice(), m_pVulkanCommandPool->GetCommandPool(), 1, &commandBuffer);
}

void VulkanProject::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
}

void VulkanProject::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

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

    EndSingleTimeCommands(commandBuffer);
}

bool VulkanProject::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}