#pragma once

#include <vector>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utils/utils.h"

class Texture;
class VulkanDescriptorSet;
class VulkanDevice;
class VulkanDescriptorPool;
class VulkanDescriptorSetLayout;
class UniformBuffer;
class VertexBuffer;
class Material;
class MaterialBuffer;

class Model final
{
public:
	struct MeshData 
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		uint32_t materialIndex = 0;
		uint32_t firstIndex = 0;
		uint32_t vertexOffset = 0;
	};

	Model();
	~Model();

	void CreateDescriptorSets(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool, VulkanDescriptorSetLayout* pLayout, 
		UniformBuffer* pUniformBuffer, VertexBuffer* pVertexBuffer, MaterialBuffer* pMaterialBuffer);

	void AddVertices(const std::vector<Vertex>& vertices);
	void AddIndices(const std::vector<uint32_t>& indices);
	void AddMesh(MeshData mesh);
	void AddTexture(std::shared_ptr<Texture> texture);
	uint32_t AddMaterial(std::shared_ptr<Material> pMaterial);
	void AddDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet);

	void SetSubMeshOffsets(size_t meshIndex, uint32_t vertexOffset, uint32_t firstIndex);

	std::vector<uint32_t> GetIndices() const;
	std::vector<Vertex> GetVertices() const;
	const std::vector<MeshData>& GetSubMeshes() const;
	const std::vector<std::unique_ptr<VulkanDescriptorSet>>& GetDescriptorSets() const;
	std::vector<std::shared_ptr<Texture>> GetAllTextures() const;
	std::vector<std::shared_ptr<Material>> GetMaterials() const;

private:
	std::vector<uint32_t> m_Indices;
	std::vector<Vertex> m_Vertices;
	std::vector<MeshData> m_Meshes;

	std::vector<std::unique_ptr<VulkanDescriptorSet>> m_pVulkanDescriptorSets;
	std::vector<std::shared_ptr<Texture>> m_pAllTextures;
	std::vector<std::shared_ptr<Material>> m_pMaterials;
};