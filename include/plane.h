#ifndef PLANE_H
#define PLANE_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};*/

class Plane 
{
public:

	Plane(const int PLANE_SIZE) {
		const int VERTEX_COUNT = PLANE_SIZE + 1;
		const float TILE_SIZE = 1.0f;

		for (int z = 0; z < VERTEX_COUNT; ++z) {
			for (int x = 0; x < VERTEX_COUNT; ++x) {
				float xPos = x * TILE_SIZE;
				float zPos = z * TILE_SIZE;

				/* vertices.push_back(xPos);
				vertices.push_back(0.0f);
				vertices.push_back(zPos);

				vertices.push_back(0.0f);
				vertices.push_back(1.0f);
				vertices.push_back(0.0f);

				vertices.push_back(x);
				vertices.push_back(z);

				vertices.push_back(0.0f);
				vertices.push_back(0.0f);
				vertices.push_back(0.0f);

				vertices.push_back(0.0f);
				vertices.push_back(0.0f);
				vertices.push_back(0.0f);*/

				Vertex vtx{};

				vtx.position =
					glm::vec3(xPos, 0.0f, zPos);

				vtx.normal =
					glm::vec3(0.0f, 1.0f, 0.0f);
				
				vtx.texCoords = glm::vec2(x, z);
				
				vtx.tangent = glm::vec3(0.0f);
				
				vtx.bitangent = glm::vec3(0.0f);

				vertices.push_back(vtx);
			}
		}

		//indices
		for (int z = 0; z < PLANE_SIZE; ++z) {
			for (int x = 0; x < PLANE_SIZE; ++x) {
				int start = z * VERTEX_COUNT + x;

				indices.push_back(start);
				indices.push_back(start + VERTEX_COUNT);
				indices.push_back(start + 1);

				indices.push_back(start + 1);
				indices.push_back(start + VERTEX_COUNT);
				indices.push_back(start + VERTEX_COUNT + 1);
			}
		}
	}

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

#endif
