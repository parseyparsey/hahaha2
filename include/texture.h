#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

class Texture
{
public:
	unsigned int ID;

	Texture(const char* texturePath, bool isFlipped, bool isGammaCorrected, int pixAllign = 4);

	inline static float tRawMem = 0.0f;
	inline static float tEstMemWMip = 0.0f;
};

#endif
