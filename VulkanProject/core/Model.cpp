#include "Model.h"
#include "Texture.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSetLayout.h"
#include "Material.h"
#include "utils/utils.h"
#include "buffers/UniformBuffer.h"
#include "buffers/VertexBuffer.h"
#include "buffers/MaterialBuffer.h"

Model::Model() :
    m_Indices{},
    m_Vertices{}
{
}

Model::~Model()
{
}

void Model::CreateDescriptorSets(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool, VulkanDescriptorSetLayout* pLayout, 
    UniformBuffer* pUniformBuffer, VertexBuffer* pVertexBuffer, MaterialBuffer* pMaterialBuffer)
{
    std::vector<Texture*> rawTextures;
    rawTextures.reserve(m_pAllTextures.size());
    for (auto& texPtr : m_pAllTextures)
    {
        rawTextures.push_back(texPtr.get());
    }

    // Build material GPU data
    std::vector<MaterialGPU> materialData = {};
    for (const auto& pMaterial : m_pMaterials)
    {
        materialData.push_back(pMaterial->BuildMaterialGPU());
    }

    pMaterialBuffer->CreateMaterialBuffer(materialData);

    // Create descriptor set(s)
    auto descriptorSet{ std::make_unique<VulkanDescriptorSet>(pDevice, pDescriptorPool) };
    descriptorSet->Create(pLayout, pUniformBuffer, pVertexBuffer, pMaterialBuffer, rawTextures);
    AddDescriptorSet(std::move(descriptorSet));
}

void Model::AddVertices(const std::vector<Vertex>& vertices)
{
	m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
}

void Model::AddIndices(const std::vector<uint32_t>& indices)
{
	m_Indices.insert(m_Indices.end(), indices.begin(), indices.end());
}

void Model::AddMesh(MeshData mesh)
{
	m_Meshes.push_back(mesh);
}

void Model::AddTexture(std::shared_ptr<Texture> texture)
{
    texture->SetBindlessIndex(static_cast<uint32_t>(m_pAllTextures.size()));
    m_pAllTextures.push_back(texture);
}

uint32_t Model::AddMaterial(std::shared_ptr<Material> pMaterial)
{
    uint32_t materialIdx{ static_cast<uint32_t>(m_pMaterials.size()) };
    m_pMaterials.push_back(pMaterial);
    
    return materialIdx;
}

void Model::AddDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet)
{
	m_pVulkanDescriptorSets.push_back(std::move(descriptorSet));
}

void Model::SetSubMeshOffsets(size_t meshIndex, uint32_t vertexOffset, uint32_t firstIndex)
{
	m_Meshes[meshIndex].vertexOffset = vertexOffset;
	m_Meshes[meshIndex].firstIndex = firstIndex;
}

std::vector<uint32_t> Model::GetIndices() const
{
    return m_Indices;
}

std::vector<Vertex> Model::GetVertices() const
{
    return m_Vertices;
}

const std::vector<Model::MeshData>& Model::GetSubMeshes() const
{
	return m_Meshes;
}

const std::vector<std::unique_ptr<VulkanDescriptorSet>>& Model::GetDescriptorSets() const
{
    return m_pVulkanDescriptorSets;
}

std::vector<std::shared_ptr<Texture>> Model::GetAllTextures() const
{
    return m_pAllTextures;
}