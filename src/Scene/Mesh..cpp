#include "Mesh.h"
#include <utility>

Mesh::Mesh(const std::vector<Vertex> &vertices) { setup(vertices); }

Mesh::~Mesh() { destroy(); }

Mesh::Mesh(Mesh &&other) noexcept { *this = std::move(other); }

Mesh &Mesh::operator=(Mesh &&other) noexcept {
	if (this != &other) {
		destroy();
		m_vao = other.m_vao;
		m_vbo = other.m_vbo;
		m_vertexCount = other.m_vertexCount;
		other.m_vao = other.m_vbo = 0;
		other.m_vertexCount = 0;
	}
	return *this;
}

void Mesh::setup(const std::vector<Vertex> &vertices) {
	m_vertexCount = (int)vertices.size();

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
				 vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // aPos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, position));

	glEnableVertexAttribArray(1); // aNormal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2); // aTexCoord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, texCoords));

	glEnableVertexAttribArray(3); // aTangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, tangent));

	glEnableVertexAttribArray(4); // aBitangent
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, bitangent));

	glBindVertexArray(0);
}

void Mesh::destroy() {
	if (m_vbo)
		glDeleteBuffers(1, &m_vbo);
	if (m_vao)
		glDeleteVertexArrays(1, &m_vao);
	m_vao = m_vbo = 0;
}

void Mesh::draw() const {
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
	glBindVertexArray(0);
}