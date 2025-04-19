#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "VulkanBuffer.h"
#include "core/utils.h"

class VulkanDevice;
class VulkanCommandPool;

class VertexBuffer final : public VulkanBuffer
{
public:
	VertexBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool);
	~VertexBuffer();

	void CreateVertexBuffer(std::vector<Vertex> vertices);

private:

};