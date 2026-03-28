#include "Material.h"
#include "Texture.h"

Material::Material(std::shared_ptr<Texture> pDiffuse, std::shared_ptr<Texture> pNormal, std::shared_ptr<Texture> pRoughness, std::shared_ptr<Texture> pMetallic) :
	m_pTextures{}
{
    if (pDiffuse)
    {
        m_pTextures.push_back(pDiffuse);
    }
    if (pNormal)
    {
        m_pTextures.push_back(pNormal);
    }
    if (pRoughness)
    {
        m_pTextures.push_back(pRoughness);
    }
	if (pMetallic)
	{
		m_pTextures.push_back(pMetallic);
	}
}

MaterialGPU Material::BuildMaterialGPU() const
{
    uint32_t fallbackIndex{ 0 };
    for (const auto& pTexture : m_pTextures)
    {
        if (pTexture->GetType() == TextureType::Unknown)
        {
            fallbackIndex = pTexture->GetBindlessIndex();
            break;
        }
    }

    MaterialGPU material{};
    material.textureIndices = glm::uvec4(fallbackIndex);

    for (const auto& pTexture : m_pTextures)
    {
        switch (pTexture->GetType())
        {
        case TextureType::Albedo:    
            material.textureIndices.r = pTexture->GetBindlessIndex(); 
            break;
        case TextureType::Normal:    
            material.textureIndices.g = pTexture->GetBindlessIndex(); 
            break;
        case TextureType::Roughness: 
            material.textureIndices.b = pTexture->GetBindlessIndex(); 
            break;
        case TextureType::Metallic:  
            material.textureIndices.a = pTexture->GetBindlessIndex(); 
            break;
        case TextureType::Unknown:
            break;
        default: 
            break;
        }
    }

    return material;
}

const std::vector<std::shared_ptr<Texture>>& Material::GetTextures() const
{
	return m_pTextures;
}
