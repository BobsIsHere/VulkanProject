#pragma once
#include <memory>
#include <vector>
#include "utils/utils.h"

class Texture;

class Material final
{
public:
	Material(std::shared_ptr<Texture> pDiffuse, std::shared_ptr<Texture> pNormal = nullptr, std::shared_ptr<Texture> pRoughness = nullptr, 
		std::shared_ptr<Texture> pMetallic = nullptr, AlphaMode alphaMode = AlphaMode::Opaque);
	~Material() = default;

	MaterialGPU BuildMaterialGPU() const;

	const std::vector<std::shared_ptr<Texture>>& GetTextures() const;
	AlphaMode GetAlphaMode() const;

private:
	std::vector<std::shared_ptr<Texture>> m_pTextures;
	AlphaMode m_AlphaMode;
};