#define TINYOBJLOADER_IMPLEMENTATION
#include <3rdParty/tiny_obj_loader.h>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

#include "ModelLoader.h"
#include "Model.h"
#include "Texture.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanDescriptorPool.h"
#include "VulkanCommandPool.h"
#include "pipelines/GraphicsPipeline.h"
#include "buffers/UniformBuffer.h"

ModelLoader::ModelLoader(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool, VulkanCommandPool* pCommandPool) :
	m_pDevice{ pDevice },
	m_pDescriptorPool{ pDescriptorPool },
	m_pCommandPool{ pCommandPool }
{
}

std::unique_ptr<Model> ModelLoader::TinyOBJLoadModel(const std::string& fileName)
{
    std::unique_ptr<Model> model{ std::make_unique<Model>() };

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning{}, error{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, fileName.c_str()))
    {
        throw std::runtime_error(warning + error);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    std::vector<Vertex> vertices;

    for (const auto& shape : shapes)
    {
        Model::MeshData meshData{};

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            meshData.indices.push_back(uniqueVertices[vertex]);
        }

        model->AddMesh(meshData);
    }

    model->AddVertices(vertices);

    return model;
}

std::unique_ptr<Model> ModelLoader::AssimpLoadModel(const std::string& fileName, GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer)
{
    std::unique_ptr<Model> model{ std::make_unique<Model>() };

    Assimp::Importer importer{};
    const aiScene* scene = importer.ReadFile(fileName,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals
    );

    if (!scene || !scene->mRootNode)
    {
        throw std::runtime_error("Failed to load model: " + fileName);
    }

    ProcessNode(model.get(), scene->mRootNode, scene, fileName);

    const auto& subMeshes{ model->GetSubMeshes() };

    for (size_t idx = 0; idx < subMeshes.size(); ++idx)
    {
        uint32_t vertexOffset{ static_cast<uint32_t>(model->GetVertices().size()) };
        uint32_t indexOffset{ static_cast<uint32_t>(model->GetIndices().size()) };

        const auto& subMesh{ subMeshes[idx] };

        model->AddVertices(subMesh.vertices);
        model->AddIndices(subMesh.indices);

        model->SetSubMeshOffsets(idx, vertexOffset, indexOffset);
    }

    return model;
}

void ModelLoader::ProcessNode(Model* model, aiNode* node, const aiScene* scene, const std::string& fileName)
{
    // Process all meshes at this node
    for (unsigned int idx = 0; idx < node->mNumMeshes; ++idx)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[idx]];
        model->AddMesh(ProcessMesh(model, mesh, scene, fileName));
    }

    // Process children
    for (unsigned int idx = 0; idx < node->mNumChildren; ++idx)
    {
        ProcessNode(model, node->mChildren[idx], scene, fileName);
    }
}

Model::MeshData ModelLoader::ProcessMesh(Model* model, aiMesh* mesh, const aiScene* scene, const std::string& fileName)
{
    Model::MeshData meshData{};

    for (unsigned int idx = 0; idx < mesh->mNumVertices; ++idx)
    {
        Vertex vertex{};

        // Position
        vertex.pos = {
            mesh->mVertices[idx].x,
            mesh->mVertices[idx].y,
            mesh->mVertices[idx].z,
            1.0f
        };

        // Normal as color for now
        if (mesh->HasNormals()) 
        {
            vertex.color = {
                (mesh->mNormals[idx].x + 1.0f) * 0.5f,
                (mesh->mNormals[idx].y + 1.0f) * 0.5f,
                (mesh->mNormals[idx].z + 1.0f) * 0.5f,
                1.0f
            };
        }
        else 
        {
            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        // Texcoords
        if (mesh->mTextureCoords[0]) 
        {
            vertex.texCoord = {
                mesh->mTextureCoords[0][idx].x,
                mesh->mTextureCoords[0][idx].y
            };
        }
        else 
        {
            vertex.texCoord = { 0.0f, 0.0f };
        }

        meshData.vertices.push_back(vertex);
    }

    // Indices
    for (unsigned int faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
    {
        const aiFace face{ mesh->mFaces[faceIdx] };

        for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
        {
            meshData.indices.push_back(face.mIndices[idx]);
        }
    }

    // Textures
    meshData.textureIndex = 0;

    if (mesh->mMaterialIndex >= 0)
    {
        const aiMaterial* material{ scene->mMaterials[mesh->mMaterialIndex] };
        aiString str;

        if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &str) == AI_SUCCESS ||
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS)
        {
			const auto pAllTextures{ model->GetAllTextures() };

            const std::string directory{ fileName.substr(0, fileName.find_last_of("/\\")) };
            std::string texPath{ str.C_Str() };
            texPath = directory + "/" + texPath;

            // Check if already loaded
            auto textureIterator{ std::find_if(pAllTextures.begin(), pAllTextures.end(),
                [&](const std::shared_ptr<Texture>& t)
                {
                    return t->GetFileName() == texPath;
                })
            };

            uint32_t texIndex{};
            if (textureIterator == pAllTextures.end())
            {
                const std::shared_ptr<Texture> pTexture{ std::make_shared<Texture>(m_pDevice, m_pCommandPool, texPath) };
                pTexture->CreateTextureImage();
                pTexture->CreateTextureImageView();
                pTexture->CreateTextureSampler();

                texIndex = static_cast<uint32_t>(pAllTextures.size());
                model->AddTexture(pTexture);
            }
            else 
            {
                texIndex = static_cast<uint32_t>(std::distance(pAllTextures.begin(), textureIterator));
            }

            meshData.textureIndex = texIndex;
        }
    }

    return meshData;
}