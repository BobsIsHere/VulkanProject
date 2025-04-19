#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "VulkanBuffer.h"
#include "core/utils.h"

class VulkanDevice;
class VulkanCommandPool;

class IndexBuffer final : public VulkanBuffer
{
public:
	IndexBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);
	~IndexBuffer();

	void CreateIndexBuffer(std::vector<uint32_t> indices);
};