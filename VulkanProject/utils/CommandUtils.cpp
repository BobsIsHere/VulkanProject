#include "CommandUtils.h"

namespace CommandUtils
{
	VkCommandBuffer CommandUtils::BeginSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool)
	{
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
	}

	void CommandUtils::EndSingleTimeCommands(VulkanDevice* device, VulkanCommandPool* commandPool, VkCommandBuffer commandBuffer)
	{
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device->GetGraphicsQueue());

        vkFreeCommandBuffers(device->GetDevice(), commandPool->GetCommandPool(), 1, &commandBuffer);
	}
}