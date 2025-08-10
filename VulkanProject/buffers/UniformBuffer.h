#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "VulkanBuffer.h"
#include "utils/utils.h"

class VulkanDevice;
class VulkanCommandPool;
class VulkanSwapChain;
class Camera;

class UniformBuffer final : public VulkanBuffer
{
public:
	UniformBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);
	~UniformBuffer();

	void CreateUniformBuffer();
	void UpdateUniformBuffer(VulkanSwapChain* pSwapChain, Camera* pCamera);

	void* GetBufferMapped() const;

private:
	void* m_pBufferMapped;
};