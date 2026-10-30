#ifndef H32R_FUNC_H
#define H32R_FUNC_H

#include <stb_image.h>

namespace h32rf {
	inline unsigned int loadCubemap(std::vector<std::string> faces) {
		unsigned int cubemapID;
		glGenTextures(1, &cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		/*
		__HDR DEBUG NOTE__
		SWITCH GAMMA CORRECTION ON FOR DIFFUSE/ALBEDO TEXTURES
		EVERYTHING ELSE STAYS OFF!!!
		*/

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char *data =
				stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

			GLenum cmformat = GL_RGB;
			if (nrChannels == 1)
				cmformat = GL_RED;
			else if (nrChannels == 3)
				cmformat = GL_RGB;
			else if (nrChannels == 4)
				cmformat = GL_RGBA;

			if (data) {
				// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
				//              width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

				if (nrChannels == 3)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB,
								 width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				else if (nrChannels == 4)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
								 GL_SRGB_ALPHA, width, height, 0, GL_RGBA,
								 GL_UNSIGNED_BYTE, data);

				stbi_image_free(data);
			} else {
				std::cout << "Cubemap failed to load at path: " << faces[i]
						  << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return cubemapID;
	}

	static std::vector<Vertex> makeSkyboxVertices() {
		static const float p[] = {
			-1, 1,	-1, -1, -1, -1, 1,	-1, -1, 1,	-1, -1, 1,	1,	-1, -1,
			1,	-1, -1, -1, 1,	-1, -1, -1, -1, 1,	-1, -1, 1,	-1, -1, 1,
			1,	-1, -1, 1,	1,	-1, -1, 1,	-1, 1,	1,	1,	1,	1,	1,	1,
			1,	1,	-1, 1,	-1, -1, -1, -1, 1,	-1, 1,	1,	1,	1,	1,	1,
			1,	1,	1,	-1, 1,	-1, -1, 1,	-1, 1,	-1, 1,	1,	-1, 1,	1,
			1,	1,	1,	1,	-1, 1,	1,	-1, 1,	-1, -1, -1, -1, -1, -1, 1,
			1,	-1, -1, 1,	-1, -1, -1, -1, 1,	1,	-1, 1};
		std::vector<Vertex> v(36);
		for (int i = 0; i < 36; i++)
			v[i].position = glm::vec3(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]);
		return v;
	}

	inline std::vector<Vertex> makeCubeVertices() {
		// position, normal, texcoord — tangent/bitangent zeroed (unused without
		// normal mapping)
		float raw[] = {// positions          // normals           // texcoords
					   -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,
					   0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 0.0f,
					   0.5f,  0.5f,	 -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
					   0.5f,  0.5f,	 -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
					   -0.5f, 0.5f,	 -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 1.0f,
					   -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,

					   -0.5f, -0.5f, 0.5f,	0.0f,  0.0f,  1.0f,	 0.0f, 0.0f,
					   0.5f,  -0.5f, 0.5f,	0.0f,  0.0f,  1.0f,	 1.0f, 0.0f,
					   0.5f,  0.5f,	 0.5f,	0.0f,  0.0f,  1.0f,	 1.0f, 1.0f,
					   0.5f,  0.5f,	 0.5f,	0.0f,  0.0f,  1.0f,	 1.0f, 1.0f,
					   -0.5f, 0.5f,	 0.5f,	0.0f,  0.0f,  1.0f,	 0.0f, 1.0f,
					   -0.5f, -0.5f, 0.5f,	0.0f,  0.0f,  1.0f,	 0.0f, 0.0f,

					   -0.5f, 0.5f,	 0.5f,	-1.0f, 0.0f,  0.0f,	 1.0f, 0.0f,
					   -0.5f, 0.5f,	 -0.5f, -1.0f, 0.0f,  0.0f,	 1.0f, 1.0f,
					   -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,	 0.0f, 1.0f,
					   -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,	 0.0f, 1.0f,
					   -0.5f, -0.5f, 0.5f,	-1.0f, 0.0f,  0.0f,	 0.0f, 0.0f,
					   -0.5f, 0.5f,	 0.5f,	-1.0f, 0.0f,  0.0f,	 1.0f, 0.0f,

					   0.5f,  0.5f,	 0.5f,	1.0f,  0.0f,  0.0f,	 1.0f, 0.0f,
					   0.5f,  0.5f,	 -0.5f, 1.0f,  0.0f,  0.0f,	 1.0f, 1.0f,
					   0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f,
					   0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f,
					   0.5f,  -0.5f, 0.5f,	1.0f,  0.0f,  0.0f,	 0.0f, 0.0f,
					   0.5f,  0.5f,	 0.5f,	1.0f,  0.0f,  0.0f,	 1.0f, 0.0f,

					   -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,	 0.0f, 1.0f,
					   0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,	 1.0f, 1.0f,
					   0.5f,  -0.5f, 0.5f,	0.0f,  -1.0f, 0.0f,	 1.0f, 0.0f,
					   0.5f,  -0.5f, 0.5f,	0.0f,  -1.0f, 0.0f,	 1.0f, 0.0f,
					   -0.5f, -0.5f, 0.5f,	0.0f,  -1.0f, 0.0f,	 0.0f, 0.0f,
					   -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,	 0.0f, 1.0f,

					   -0.5f, 0.5f,	 -0.5f, 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f,
					   0.5f,  0.5f,	 -0.5f, 0.0f,  1.0f,  0.0f,	 1.0f, 1.0f,
					   0.5f,  0.5f,	 0.5f,	0.0f,  1.0f,  0.0f,	 1.0f, 0.0f,
					   0.5f,  0.5f,	 0.5f,	0.0f,  1.0f,  0.0f,	 1.0f, 0.0f,
					   -0.5f, 0.5f,	 0.5f,	0.0f,  1.0f,  0.0f,	 0.0f, 0.0f,
					   -0.5f, 0.5f,	 -0.5f, 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f};

		std::vector<Vertex> verts;
		for (int i = 0; i < 36; i++) {
			Vertex v{};
			v.position =
				glm::vec3(raw[i * 8 + 0], raw[i * 8 + 1], raw[i * 8 + 2]);
			v.normal =
				glm::vec3(raw[i * 8 + 3], raw[i * 8 + 4], raw[i * 8 + 5]);
			v.texCoords = glm::vec2(raw[i * 8 + 6], raw[i * 8 + 7]);
			v.tangent = glm::vec3(0.0f);
			v.bitangent = glm::vec3(0.0f);
			verts.push_back(v);
		}
		return verts;
	}

	inline bool isPL(const std::string &str) {
		size_t pos = str.find("pl");

		if (pos != std::string::npos) {
			if (pos + 2 < str.length()) {
				return std::isdigit(str[pos + 2]);
			}
		}
		return false;
	}

	inline void computeTangents(std::vector<Vertex> &v,
								const std::vector<unsigned int> &idx = {}) {
		for (auto &x : v)
			x.tangent = x.bitangent = glm::vec3(0.0f);

		size_t tris = (idx.empty() ? v.size() : idx.size()) / 3;
		for (size_t t = 0; t < tris; t++) {
			unsigned a = idx.empty() ? unsigned(t * 3) : idx[t * 3];
			unsigned b = idx.empty() ? unsigned(t * 3 + 1) : idx[t * 3 + 1];
			unsigned c = idx.empty() ? unsigned(t * 3 + 2) : idx[t * 3 + 2];

			glm::vec3 e1 = v[b].position - v[a].position;
			glm::vec3 e2 = v[c].position - v[a].position;
			glm::vec2 d1 = v[b].texCoords - v[a].texCoords;
			glm::vec2 d2 = v[c].texCoords - v[a].texCoords;

			float det = d1.x * d2.y - d2.x * d1.y;
			if (std::abs(det) < 1e-8f)
				continue; // degenerate UVs
			float r = 1.0f / det;
			glm::vec3 T = r * (e1 * d2.y - e2 * d1.y);
			glm::vec3 B = r * (e2 * d1.x - e1 * d2.x);

			for (unsigned i : {a, b, c}) {
				v[i].tangent += T;
				v[i].bitangent += B;
			}
		}

		for (auto &x : v) {
			glm::vec3 t =
				x.tangent -
				x.normal * glm::dot(x.normal, x.tangent); // orthogonalize
			if (glm::length(t) < 1e-6f) // no usable UV triangle
				t = glm::cross(x.normal, std::abs(x.normal.y) < 0.9f
											 ? glm::vec3(0, 1, 0)
											 : glm::vec3(1, 0, 0));
			x.tangent = glm::normalize(t);
			float h =
				glm::dot(glm::cross(x.normal, x.tangent), x.bitangent) < 0.0f
					? -1.0f
					: 1.0f;
			x.bitangent = glm::cross(x.normal, x.tangent) * h;
		}
	}

	inline void whiteTxt(unsigned int& whiteTex) {
		unsigned char white[4] = {255, 255, 255, 255};
		glGenTextures(1, &whiteTex);
		glBindTexture(GL_TEXTURE_2D, whiteTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, white);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	inline void genTexColored(unsigned int &Tex, int r, int g, int b, int a) {
		unsigned char color[4] = {r, g, b, a};
		glGenTextures(1, &Tex);
		glBindTexture(GL_TEXTURE_2D, Tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

#endif