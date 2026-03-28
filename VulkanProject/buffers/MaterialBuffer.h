#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "utils/utils.h"
#include "VulkanBuffer.h"

class VulkanDevice;
class VulkanCommandPool;

class MaterialBuffer final : public VulkanBuffer
{
public:
	MaterialBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);
	~MaterialBuffer();

	void CreateMaterialBuffer(std::vector<MaterialGPU> materials);
};