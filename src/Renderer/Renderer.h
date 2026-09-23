#pragma once
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include "Framebuffer.h"
#include "shader.h"
#include "Scene/Scene.h"

class Renderer {
public:
	Renderer(int width, int height);

	void resize(int width, int height);
	void render(Scene &scene);
private:
	void renderForward(Scene &scene);
	void bloom();
	void postFX();
	void bindMaterial(const Material &material);
	void setLightUniforms(Scene &scene);

	int m_height, m_width;

	Framebuffer m_resolveFBO;
	Framebuffer m_brightFBO;
	Framebuffer m_pingpongFBO[2];

	Shader m_lightShader;
	Shader m_brightPassShader;
	Shader m_blurShader;
	Shader m_postFXShader;

	unsigned int m_quadVAO = 0, m_quadVBO = 0;
	void initQuad();
	void drawQuad();

	bool m_hdrEnabled = true;
	float m_exposure = 1.0f;
	int m_postFXMode = 0;
	bool m_bloomEnabled = true;
};