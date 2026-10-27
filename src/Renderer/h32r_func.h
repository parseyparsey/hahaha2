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

}

#endif