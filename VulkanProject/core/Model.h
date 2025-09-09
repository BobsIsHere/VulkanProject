#pragma once

#include <vector>
#include <memory>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utils/utils.h"

class Texture;
class VulkanDescriptorSet;
class VulkanDevice;
class VulkanDescriptorPool;
class VulkanCommandPool;
class GraphicsPipeline;
class UniformBuffer;

class Model final
{
public:
	struct MeshData 
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::vector<std::shared_ptr<Texture>> textures;
		uint32_t textureIndex;
	};

	Model(std::string fileName);
	~Model();

	void TinyOBJLoadModel();
	void AssimpLoadModel(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool, VulkanCommandPool* pCommandPool, GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer);

	std::vector<uint32_t> GetIndices() const;
	std::vector<Vertex> GetVertices() const;
	std::vector<MeshData> GetSubMeshes() const;
	const std::vector<std::unique_ptr<VulkanDescriptorSet>>& GetDescriptorSets() const;

private:
	std::vector<std::unique_ptr<VulkanDescriptorSet>> m_pVulkanDescriptorSets;

	std::string m_FileName;
	std::vector<uint32_t> m_Indices;
	std::vector<Vertex> m_Vertices;
	std::vector<MeshData> m_Meshes;
	std::vector<std::shared_ptr<Texture>> m_AllTextures;

	void ProcessNode(aiNode* node, const aiScene* scene);
	MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);
};