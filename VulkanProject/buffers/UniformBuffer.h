#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "VulkanBuffer.h"
#include "core/utils.h"

class VulkanDevice;
class VulkanCommandPool;

class UniformBuffer final : public VulkanBuffer
{
public:
	UniformBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);
	~UniformBuffer();

	void CreateUniformBuffer();

private:
	void* m_pBufferMapped;
};