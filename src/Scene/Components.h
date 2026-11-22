#pragma once
#include <string>
#include <vector>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Mesh;
struct Material;

struct Name {
	std::string value;
};

struct Transform {
	glm::vec3 position{0.0f};
	glm::vec3 rotation{0.0f}; // degrees
	glm::vec3 scale{1.0f};

	glm::mat4 localMatrix() const {
		glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
		m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		return glm::scale(m, scale);
	}
};

struct WorldTransform {
	glm::mat4 matrix{1.0f};
};

struct Hierarchy {
	entt::entity parent = entt::null;
	std::vector<entt::entity> children;
};

struct Renderable {
	Mesh *mesh = nullptr;
	Material *material = nullptr;
};