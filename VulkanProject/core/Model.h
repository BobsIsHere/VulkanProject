#pragma once

#include <vector>
#include <string>

#include "utils/utils.h"

class Model final
{
public:
	Model(std::string fileName);
	~Model();

	void LoadModel();

	std::vector<uint32_t> GetIndices() const;
	std::vector<Vertex> GetVertices() const;

private:
	std::string m_FileName;
	std::vector<uint32_t> m_Indices;
	std::vector<Vertex> m_Vertices;
};