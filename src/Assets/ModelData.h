#pragma once

#include <string>
#include <vector>
#include "Scene/Mesh.h"

struct MaterialData {
	std::string diffPath, specPath, normPath;
	float shininess = 32.0f;
};

struct SubMeshData {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int materialIndex = 0;
};

struct ModelData {
	std::string directory;
	std::vector<SubMeshData> meshes;
	std::vector<MaterialData> materials;
};