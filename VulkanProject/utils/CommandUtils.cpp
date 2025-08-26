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

        VkCommandBufferSubmitInfo cmdBufSubmitInfo{};
        cmdBufSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdBufSubmitInfo.commandBuffer = commandBuffer;
        cmdBufSubmitInfo.deviceMask = 0;

        VkSubmitInfo2 submitInfo2{};
        submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo2.commandBufferInfoCount = 1;
        submitInfo2.pCommandBufferInfos = &cmdBufSubmitInfo;

        vkQueueSubmit2(device->GetGraphicsQueue(), 1, &submitInfo2, VK_NULL_HANDLE);
        vkQueueWaitIdle(device->GetGraphicsQueue());

        vkFreeCommandBuffers(device->GetDevice(), commandPool->GetCommandPool(), 1, &commandBuffer);
	}
}