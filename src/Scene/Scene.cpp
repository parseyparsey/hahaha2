#include "Scene.h"

GameObject &Scene::addObject(char const *objName, Mesh *mesh, Material *material) {
	GameObject obj;
	obj.name = objName;
	obj.mesh = mesh;
	obj.material = material;
	m_objects.push_back(obj);
	return m_objects.back();
}

Mesh &Scene::addMesh(const std::vector<Vertex> &verts) {
	m_meshes.push_back(std::make_unique<Mesh>(verts));
	return *m_meshes.back();
}

Material &Scene::addMaterial(const Material &mat) {
	m_materials.push_back(std::make_unique<Material>(mat));
	return *m_materials.back();
}
