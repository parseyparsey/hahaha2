#include "Scene.h"

GameObject &Scene::addObject(char const *objName, Mesh *mesh, Material *material) {
	GameObject obj;
	obj.name = objName;
	obj.mesh = mesh;
	obj.material = material;
	m_objects.push_back(obj);
	return m_objects.back();
}

Mesh &Scene::addMesh(const std::vector<Vertex> &verts,
					 const std::vector<unsigned int> &indices) {
	m_meshes.push_back(std::make_unique<Mesh>(verts, indices));
	return *m_meshes.back();
}

Material &Scene::addMaterial(const Material &mat) {
	m_materials.push_back(std::make_unique<Material>(mat));
	return *m_materials.back();
}

entt::entity Scene::createEntity(const std::string &name, entt::entity parent) {
	entt::entity e = m_registry.create();
	m_registry.emplace<Name>(e, Name{name});
	m_registry.emplace<Transform>(e);
	m_registry.emplace<WorldTransform>(e);
	m_registry.emplace<Hierarchy>(e);
	if (parent != entt::null)
		setParent(e, parent);
	return e;
}

void Scene::setParent(entt::entity child, entt::entity newParent) {
	// refuse cycles: the new parent can't be the child or one of its
	// descendants
	for (entt::entity p = newParent; p != entt::null;
		 p = m_registry.get<Hierarchy>(p).parent)
		if (p == child)
			return;

	auto &h = m_registry.get<Hierarchy>(child);
	if (h.parent != entt::null) {
		auto &sib = m_registry.get<Hierarchy>(h.parent).children;
		sib.erase(std::remove(sib.begin(), sib.end(), child), sib.end());
	}
	h.parent = newParent;
	if (newParent != entt::null)
		m_registry.get<Hierarchy>(newParent).children.push_back(child);
}

void Scene::setRenderable(entt::entity e, Mesh *mesh, Material *mat) {
	m_registry.emplace_or_replace<Renderable>(e, Renderable{mesh, mat});
}

void Scene::updateTransforms() {
	auto roots = m_registry.view<Hierarchy>();
	for (auto e : roots)
		if (roots.get<Hierarchy>(e).parent == entt::null)
			updateWorld(e, glm::mat4(1.0f));
}

void Scene::updateWorld(entt::entity e, const glm::mat4 &parentWorld) {
	glm::mat4 world = parentWorld * m_registry.get<Transform>(e).localMatrix();
	m_registry.get<WorldTransform>(e).matrix = world;
	for (entt::entity c : m_registry.get<Hierarchy>(e).children)
		updateWorld(c, world);
}