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

    const std::array<VkDescriptorSetLayoutBinding, 2> uboBindings = { uboBinding, ssboBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(uboBindings.size());
    layoutInfo.pBindings = uboBindings.data();

    vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &layoutInfo, nullptr, &m_UBOSetLayout);

    // --- Texture layout ---
    VkDescriptorSetLayoutBinding materialBufferBinding{};
    materialBufferBinding.binding = 0;
    materialBufferBinding.descriptorCount = 1;
    materialBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    materialBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialBufferBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampledImageBinding{};
    sampledImageBinding.binding = 2;
    sampledImageBinding.descriptorCount = utils::TEXTURE_ARRAY_SIZE;
    sampledImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    sampledImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampledImageBinding.pImmutableSamplers = nullptr;

    const std::array<VkDescriptorSetLayoutBinding, 3> globalBindings = { materialBufferBinding, samplerLayoutBinding, sampledImageBinding };

    m_BindingFlags.clear();
    m_BindingFlags.resize(globalBindings.size(), 0);

    m_BindingFlags[0] = 0;
    m_BindingFlags[1] = 0;
    m_BindingFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
	bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	bindingFlagsInfo.bindingCount = static_cast<uint32_t>(globalBindings.size());
	bindingFlagsInfo.pBindingFlags = m_BindingFlags.data();

    VkDescriptorSetLayoutCreateFlags flags = 0;
	flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    VkDescriptorSetLayoutCreateInfo globalLayoutInfo{};
    globalLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	globalLayoutInfo.pNext = &bindingFlagsInfo;
	globalLayoutInfo.flags = flags;
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
