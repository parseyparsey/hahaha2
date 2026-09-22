#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

class Mesh {
public:
	Mesh(const std::vector<Vertex>& vertices);
	~Mesh();

	Mesh(const Mesh &) = delete;
	Mesh &operator=(const Mesh &) = delete;
	Mesh(Mesh &&other) noexcept;
	Mesh &operator=(Mesh &&other) noexcept;

	void draw() const;

private:
	void setup(const std::vector<Vertex> &vertices);
	void destroy();

	unsigned m_vao = 0, m_vbo = 0;
	int m_vertexCount = 0;
};