#ifndef SPHERE_H 
#define SPHERE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Sphere {
public:
	Sphere(const int SPHERE_SIZE, const float RADIUS = 1.0f) {
		const int SECTOR_COUNT = SPHERE_SIZE;
		const int STACK_COUNT = SPHERE_SIZE;

		const float PI = 3.14159265359f;

		// Vertices
		for (int i = 0; i <= STACK_COUNT; ++i) {
			float stackAngle = PI / 2.0f - i * PI / STACK_COUNT;

			float xy = RADIUS * cosf(stackAngle);
			float y = RADIUS * sinf(stackAngle);

			for (int j = 0; j <= SECTOR_COUNT; ++j) {
				float sectorAngle = j * 2.0f * PI / SECTOR_COUNT;

				float x = xy * cosf(sectorAngle);
				float z = xy * sinf(sectorAngle);

				Vertex vtx{};

				vtx.position = glm::vec3(x, y, z);

				vtx.normal = glm::normalize(vtx.position);

				vtx.texCoords =
					glm::vec2((float)j / SECTOR_COUNT, (float)i / STACK_COUNT);

				vtx.tangent = glm::vec3(0.0f);

				vtx.bitangent = glm::vec3(0.0f);

				vertices.push_back(vtx);
			}
		}

		// Indices
		for (int i = 0; i < STACK_COUNT; ++i) {
			int k1 = i * (SECTOR_COUNT + 1);
			int k2 = k1 + SECTOR_COUNT + 1;

			for (int j = 0; j < SECTOR_COUNT; ++j) {
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (STACK_COUNT - 1)) {
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}

				++k1;
				++k2;
			}
		}
	}

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

#endif