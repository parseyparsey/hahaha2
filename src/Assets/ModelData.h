#pragma once

#include <string>
#include <vector>
#include "Scene/Mesh.h"

struct MaterialData {
	std::string diffPath, specPath, normPath, heightPath;
	float shininess = 32.0f;

	std::string metallicPath, aoPath, roughnessPath;
	std::string metalRoughPath; // glTF packed texture: G = roughness, B = metallic
	glm::vec3 baseColor{1.0f};
	float metallic = 1.0f, roughness = 1.0f;
	bool isPBR = false, packedMR = false;
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

struct TextureFlips {
	bool diffuse = true;
	bool specular = true;
	bool normal = true;
	bool height = true;
	bool pbr = true;
};