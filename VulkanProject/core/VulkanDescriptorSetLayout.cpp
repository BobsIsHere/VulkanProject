#include <stdexcept>

#include "VulkanDescriptorSetLayout.h"
#include "core/VulkanDevice.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice* pDevice) :
	m_pDevice{ pDevice },
	m_GlobalDataSetLayout{},
	m_UBOSetLayout{}
{
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
}

void VulkanDescriptorSetLayout::Create()
{
    // --- UBO layout --- 
    VkDescriptorSetLayoutBinding uboBinding{};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding ssboBinding{};
    ssboBinding.binding = 1;
    ssboBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboBinding.descriptorCount = 1;
    ssboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ssboBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> uboBindings = { uboBinding, ssboBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(uboBindings.size());
    layoutInfo.pBindings = uboBindings.data();

    vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &layoutInfo, nullptr, &m_UBOSetLayout);

    // --- Texture layout ---
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampledImageBinding{};
    sampledImageBinding.binding = 1;
    sampledImageBinding.descriptorCount = utils::TEXTURE_ARRAY_SIZE;
    sampledImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    sampledImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampledImageBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> globalBindings = { samplerLayoutBinding, sampledImageBinding };

    VkDescriptorSetLayoutCreateInfo globalLayoutInfo{};
    globalLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    globalLayoutInfo.bindingCount = static_cast<uint32_t>(globalBindings.size());
    globalLayoutInfo.pBindings = globalBindings.data();

    if (vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &globalLayoutInfo, nullptr, &m_GlobalDataSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanDescriptorSetLayout::Cleanup()
{
	vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_GlobalDataSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_UBOSetLayout, nullptr);
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::GetGlobalSetLayout() const
{
	return m_GlobalDataSetLayout;
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::GetUBOSetLayout() const
{
	return m_UBOSetLayout;
}
