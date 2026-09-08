#include <texture.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

int ti = 0;

Texture::Texture(const char* texturePath, bool isFlipped, bool isGammaCorrected, int pixAllign) {
	glGenTextures(1, &ID);

	glBindTexture(GL_TEXTURE_2D, ID);

	//test
	glPixelStorei(GL_UNPACK_ALIGNMENT, pixAllign);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (isFlipped) {
		stbi_set_flip_vertically_on_load(true);
	}

	int width, height, nrChannels;
	unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

	// VRAM estimate: raw size * 1.33 for mipmap chain overhead
	float rawMB = (width * height * nrChannels) / (1024.0f * 1024.0f);
	float estVRAM_MB = rawMB * 1.33f;

	Texture::tRawMem += rawMB;
	Texture::tEstMemWMip += estVRAM_MB;

	std::cout << "TEXTURE" << ti << ": " << texturePath << " | " << width << "x"
			  << height << " (" << nrChannels << "ch)" << " | " << rawMB << "MB"
			  << " | ~" << estVRAM_MB << " MB (w/ mips)" << std::endl;

	GLenum format = GL_RGB;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;

	if (data) {
		//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (!isGammaCorrected)
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		else {
			if (nrChannels == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (nrChannels == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "TEXTURE" << ti << ": " << "Failed to load texture." << std::endl;
	}
	ti++;

	if (isFlipped) {
		stbi_set_flip_vertically_on_load(false);
	}

	stbi_image_free(data);
}
