#define TINYOBJLOADER_IMPLEMENTATION
#include <3rdParty/tiny_obj_loader.h>

#include <iostream>

#include "Model.h"
#include "Texture.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanDescriptorPool.h"
#include "VulkanCommandPool.h"
#include "pipelines/GraphicsPipeline.h"
#include "buffers/UniformBuffer.h"
#include "utils/utils.h"

Model::Model(std::string fileName) :
    m_FileName{ fileName },
    m_Indices{},
    m_Vertices{}
{
}

Model::~Model()
{
}

void Model::TinyOBJLoadModel()
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning{};
    std::string error{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, m_FileName.c_str()))
    {
        throw std::runtime_error(warning + error);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                m_Vertices.push_back(vertex);
            }

            m_Indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void Model::AssimpLoadModel(VulkanDevice* pDevice, VulkanDescriptorPool* pDescriptorPool, VulkanCommandPool* pCommandPool, GraphicsPipeline* pPipeline, UniformBuffer* pUniformBuffer)
{
    Assimp::Importer importer{};
    const aiScene* scene = importer.ReadFile(m_FileName,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals
    );

    if (!scene || !scene->mRootNode)
    {
        throw std::runtime_error("Failed to load model: " + m_FileName);
    }

    ProcessNode(scene->mRootNode, scene, pDevice, pCommandPool);

    for (auto& subMesh : m_Meshes)
    {
        uint32_t vertexOffset = static_cast<uint32_t>(m_Vertices.size());
        uint32_t indexOffset = static_cast<uint32_t>(m_Indices.size());

        m_Vertices.insert(m_Vertices.end(), subMesh.vertices.begin(), subMesh.vertices.end());
        m_Indices.insert(m_Indices.end(), subMesh.indices.begin(), subMesh.indices.end());

        subMesh.vertexOffset = vertexOffset;
        subMesh.firstIndex = indexOffset;
    }

    std::vector<Texture*> rawTextures;
    rawTextures.reserve(m_AllTextures.size());
    for (auto& texPtr : m_AllTextures)
    {
        rawTextures.push_back(texPtr.get());
    }

    // Create descriptor set(s)
    m_pVulkanDescriptorSets.push_back(std::make_unique<VulkanDescriptorSet>(pDevice, pDescriptorPool));
    m_pVulkanDescriptorSets.back()->Create(pPipeline, pUniformBuffer, rawTextures);
}

std::vector<uint32_t> Model::GetIndices() const
{
    return m_Indices;
}

std::vector<Vertex> Model::GetVertices() const
{
    return m_Vertices;
}

std::vector<Model::MeshData> Model::GetSubMeshes() const
{
    return m_Meshes;
}

const std::vector<std::unique_ptr<VulkanDescriptorSet>>& Model::GetDescriptorSets() const
{
    return m_pVulkanDescriptorSets;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, VulkanDevice* pDevice, VulkanCommandPool* pCommandPool)
{
    // Process all meshes at this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_Meshes.push_back(ProcessMesh(mesh, scene, pDevice, pCommandPool));
    }

    // Process children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, pDevice, pCommandPool);
    }
}

Model::MeshData Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, VulkanDevice* pDevice, VulkanCommandPool* pCommandPool)
{
    MeshData meshData{};

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};

        // Position
        vertex.pos = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        // Normal as color for now
        if (mesh->HasNormals()) {
            vertex.color = {
                (mesh->mNormals[i].x + 1.0f) * 0.5f,
                (mesh->mNormals[i].y + 1.0f) * 0.5f,
                (mesh->mNormals[i].z + 1.0f) * 0.5f
            };
        }
        else {
            vertex.color = { 1.0f, 1.0f, 1.0f };
        }

        // Texcoords (optional)
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }
        else {
            vertex.texCoord = { 0.0f, 0.0f };
        }

        meshData.vertices.push_back(vertex);
    }

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            meshData.indices.push_back(face.mIndices[j]);
        }
    }

    // Textures
    meshData.textureIndex = 0;

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString str;

        if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &str) == AI_SUCCESS ||
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS) // fallback
        {
            std::string texPath = str.C_Str();
            std::string directory = m_FileName.substr(0, m_FileName.find_last_of("/\\"));
            texPath = directory + "/" + texPath;

            // Check if already loaded
            auto it = std::find_if(m_AllTextures.begin(), m_AllTextures.end(),
                [&](const std::shared_ptr<Texture>& t) { return t->GetFileName() == texPath; });

            uint32_t texIndex;
            if (it == m_AllTextures.end()) {
                auto texture = std::make_shared<Texture>(pDevice, pCommandPool, texPath);
                texture->CreateTextureImage();
                texture->CreateTextureImageView();
                texture->CreateTextureSampler();

                texIndex = static_cast<uint32_t>(m_AllTextures.size());
                m_AllTextures.push_back(texture);
            }
            else {
                texIndex = static_cast<uint32_t>(std::distance(m_AllTextures.begin(), it));
            }

            meshData.textureIndex = texIndex;
        }
    }

    return meshData;
}