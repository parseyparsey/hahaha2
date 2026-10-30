#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include "Mesh.h"

enum ShadingModel {
	BLINNPHONG = 0, 
	PBR = 1
};

struct Material {
	int model = ShadingModel::BLINNPHONG;

	unsigned int diff = 0;
	unsigned int parallaxMap = 0;
	unsigned int norm = 0;
	float heightScale = 0.1f;
	bool parallaxClip = false;
	
	bool isNormalMapped() const { return norm != 0; }
	bool isParallaxMapped() const { return parallaxMap != 0; }
	
	//BLINN-PHONG
	unsigned int spec = 0;
	float shininess = 32.0f;

	//PBR
	unsigned int metallicMap = 0, aoMap = 0, emissiveMap = 0, roughnessMap = 0;
	float metallic = 1.0f, roughness = 1.0f;
};

struct Transform {
	glm::vec3 position{0.0f};
	glm::vec3 rotation{0.0f};
	glm::vec3 scale{1.0f};
};

class GameObject {
public:
	std::string name;
	Transform transform;
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	bool active = true;

	glm::mat4 getModelMatrix() const;
};