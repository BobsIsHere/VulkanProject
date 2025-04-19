#pragma once

#include "core/VulkanDevice.h"
#include "core/VulkanCommandPool.h"
#include "core/VulkanCommandBuffer.h"

namespace CommandUtils
{
	VkCommandBuffer BeginSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool);
	void EndSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool, VkCommandBuffer commandBuffer);
}