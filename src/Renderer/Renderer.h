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
#include "h32r_func.h"

class Renderer {
public:
	Renderer(int width, int height);

	void resize(int width, int height);
	void render(Scene &scene);
private:
	void renderForward(Scene &scene);
	void renderDirShadow(Scene &scene);
	void renderPointShadow(Scene &scene);
	void renderSpotShadow(Scene &scene);
	glm::mat4 spotLightSpaceMatrix(SpotLight &sl);
	void bloom();
	void postFX();
	void bindMaterial(const Material &material);
	void setLightUniforms(Scene &scene);
	void renderSkybox(Scene &scene);

	int m_height, m_width;

	static constexpr int m_dirShadowMapSize = 4096;
	static constexpr int m_pointShadowMapSize = 2048;
	static constexpr int m_spotShadowMapSize = 2048;

	static constexpr int MAX_POINT_SHADOW = 2;
	float m_pointFarPlane = 25.0f;
	int m_pointShadowCount;

	Framebuffer m_dirShadowDepthFBO;
	Framebuffer m_resolveFBO;
	Framebuffer m_brightFBO;
	Framebuffer m_pingpongFBO[2];
	Framebuffer m_ldrFBO;
	std::vector<Framebuffer> m_pointShadowFBO;

	std::vector<Framebuffer> m_spotShadowDepthFBO;
	std::vector<glm::mat4> m_spotLightSpaceMatrices;

	Shader m_lightShader;
	Shader m_brightPassShader;
	Shader m_blurShader;
	Shader m_postFXShader;
	Shader m_debugShader;
	Shader m_dirShadowDepthShader;
	Shader m_pointShadowDepthShader;
	Shader m_skyboxShader;
	Shader m_unlitShader;
	Shader m_fxaaShader;

	unsigned int m_quadVAO = 0, m_quadVBO = 0;
	void initQuad();
	void drawQuad();

	bool m_hdrEnabled = true;
	float m_exposure = 1.0f;
	int m_postFXMode = 0;
	bool m_bloomEnabled = true;
	bool m_fxaaEnabled = true;

	static constexpr int MAX_SPOT_LIGHTS = 4;

	Mesh m_skyboxMesh;

	enum TextureUnit {
		Diffuse = 0,
		Specular = 1,
		DirShadowMap = 4,
		PointShadowMap = 5,
		NormalMap = 7,
		ParallaxMap = 8,
		SpotShadowBase = 9,
		SkyboxMap = 13
	};

	glm::mat4 lightSpaceMatrix;
};