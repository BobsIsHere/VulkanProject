#include "Model.h"
#include "Texture.h"
#include "VulkanDescriptorSet.h"
#include "utils/utils.h"

Model::Model() :
    m_Indices{},
    m_Vertices{}
{
}

Model::~Model()
{
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
	m_pAllTextures.push_back(texture);
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