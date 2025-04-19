#include <chrono>

#include "UniformBuffer.h"
#include "core/VulkanDevice.h"
#include "core/VulkanCommandPool.h"
#include "core/VulkanSwapChain.h"

UniformBuffer::UniformBuffer(VulkanDevice* pDevice, VulkanCommandPool* pCommandPool) :
	VulkanBuffer(pDevice, pCommandPool),
	m_pBufferMapped{}
{
}

UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::CreateUniformBuffer()
{
	VkDeviceSize bufferSize{ sizeof(UniformBufferObject) };

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffer, m_BufferMemory);

	vkMapMemory(m_pVulkanDevice->GetDevice(), m_BufferMemory, 0, bufferSize, 0, &m_pBufferMapped);
}

void UniformBuffer::UpdateUniformBuffer(VulkanSwapChain* pSwapChain)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), pSwapChain->GetSwapChainExtent().width /
        (float)pSwapChain->GetSwapChainExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_pBufferMapped, &ubo, sizeof(ubo));
}

void* UniformBuffer::GetBufferMapped() const
{
	return m_pBufferMapped;
}
