#include <chrono>

#include "UniformBuffer.h"
#include "core/VulkanDevice.h"
#include "core/VulkanCommandPool.h"
#include "core/VulkanSwapChain.h"
#include "core/Camera.h"

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

void UniformBuffer::UpdateUniformBuffer(VulkanSwapChain* pSwapChain, Camera* pCamera)
{
	const float m_FOV{ 45.f };
	const float m_AspectRatio{ pSwapChain->GetSwapChainExtent().width / static_cast<float>(pSwapChain->GetSwapChainExtent().height) };

	UniformBufferObject ubo{};
	ubo.model = glm::mat4(1.0f);
	ubo.view = pCamera->Update();
	ubo.proj = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 0.1f, 5000.f);
	ubo.proj[1][1] *= -1;

    memcpy(m_pBufferMapped, &ubo, sizeof(ubo));
}

void* UniformBuffer::GetBufferMapped() const
{
	return m_pBufferMapped;
}
