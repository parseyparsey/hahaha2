#ifndef PLANE_H
#define PLANE_H

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <texture.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Plane 
{
public:

	Plane(const int PLANE_SIZE) {
		const int VERTEX_COUNT = PLANE_SIZE + 1;
		const float TILE_SIZE = 1.0f;
		planetxt =  new Texture("textures/wood.png"/*"textures/plane00.jpg"*/, false, true);
		planetxt_specular = new Texture("textures/white.png", false, false);

		for (int z = 0; z < VERTEX_COUNT; ++z) {
			for (int x = 0; x < VERTEX_COUNT; ++x) {
				float xPos = x * TILE_SIZE;
				float zPos = z * TILE_SIZE;

				vertices.push_back(xPos);
				vertices.push_back(0.0f);
				vertices.push_back(zPos);

				vertices.push_back(0.0f);
				vertices.push_back(1.0f);
				vertices.push_back(0.0f);

				vertices.push_back(x);
				vertices.push_back(z);
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

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		int STRIDE = 8 * sizeof(float);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*) 0);
		glEnableVertexAttribArray(0);
		
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*) (3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, STRIDE, (void*) (6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	void draw(Shader& shader) {
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planetxt->ID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, planetxt_specular->ID);
		shader.setInt("material.texture_diffuse", 0);
		shader.setInt("material.texture_specular", 1);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

private:
	unsigned int VAO, VBO, EBO;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	Texture* planetxt;
	Texture* planetxt_specular;
};

#endif
