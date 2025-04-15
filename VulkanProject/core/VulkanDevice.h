#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

private:
	VkDevice device;
};