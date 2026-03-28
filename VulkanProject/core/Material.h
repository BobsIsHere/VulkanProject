#pragma once
#include <memory>
#include <vector>
#include "utils/utils.h"

class Texture;

class Material final
{
public:
	Material(std::shared_ptr<Texture> pDiffuse, std::shared_ptr<Texture> pNormal, std::shared_ptr<Texture> pRoughness, std::shared_ptr<Texture> pMetallic = nullptr);
	~Material() = default;

	MaterialGPU BuildMaterialGPU() const;

	const std::vector<std::shared_ptr<Texture>>& GetTextures() const;

private:
	std::vector<std::shared_ptr<Texture>> m_pTextures;
};