#pragma once

#include "core/VulkanDevice.h"
#include "core/VulkanCommandPool.h"

namespace CommandUtils
{
	VkCommandBuffer BeginSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool);
	void EndSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool, VkCommandBuffer commandBuffer);
}