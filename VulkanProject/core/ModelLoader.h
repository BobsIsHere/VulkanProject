#pragma once

#include <string>
#include <vector>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"

class VulkanDevice;
class VulkanDescriptorPool;
class VulkanCommandPool;
class VulkanDescriptorSet;
class GraphicsPipeline;
class UniformBuffer;

class ModelLoader final
{
public:
	ModelLoader(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool, VulkanCommandPool* pCommandPool);
	~ModelLoader() = default;

	std::unique_ptr<Model> TinyOBJLoadModel(const std::string& fileName);
	std::unique_ptr<Model> AssimpLoadModel(const std::string& fileName, GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer);

private:
	VulkanDevice* m_pDevice;
	VulkanDescriptorPool* m_pDescriptorPool;
	VulkanCommandPool* m_pCommandPool;
	std::vector<std::unique_ptr<VulkanDescriptorSet>> m_pVulkanDescriptorSets;

	void ProcessNode(Model* model, aiNode* node, const aiScene* scene, const std::string& fileName);
	Model::MeshData ProcessMesh(Model* model, aiMesh* mesh, const aiScene* scene, const std::string& fileName);
};