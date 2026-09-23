#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GameObject.h"
#include "Mesh.h"
#include <vector>
#include <memory>

struct PointLight {
	glm::vec3 position;
	float constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
	glm::vec3 ambient, diffuse, specular;
};

struct DirLight {
	glm::vec3 direction;
	glm::vec3 ambient, diffuse, specular;
};

struct SpotLight {
	glm::vec3 position, direction;
	glm::vec3 ambient, diffuse, specular;
	float cutoff = 0.0f, outerCutoff = 0.0f;
	float constant = 1.0f, linear = 0.0f, quadratic = 0.0f;
};

class Scene {
public:
	GameObject &addObject(char const *objName, Mesh *mesh, Material *material);

	std::vector<GameObject> &getObjects() { return m_objects; }
	std::vector<PointLight> &getPointLights() { return m_pointLights; }
	DirLight &getDirLight() { return m_dirLight; }
	SpotLight &getSpotLight() { return m_spotLight; }

	Mesh &addMesh(const std::vector<Vertex> &verts);
	Material &addMaterial(const Material &mat);

private:
	std::vector<GameObject> m_objects;
	std::vector<PointLight> m_pointLights;
	DirLight m_dirLight;
	SpotLight m_spotLight;

	std::vector<std::unique_ptr<Material>> m_materials;
	std::vector<std::unique_ptr<Mesh>> m_meshes;
};
