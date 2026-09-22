#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include "Mesh.h"
#include "Material.h"

struct Material {
	unsigned int diff = 0;
	unsigned int spec = 0;
	unsigned int norm = 0;
	unsigned int parallaxMap = 0;
	bool isNormalMapped = false;
	bool isParallaxMapped = false;
	float shininess = 32.0f;
	float heightScale = 0.1f;
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
};