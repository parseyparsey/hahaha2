#include "Renderer.h"

Renderer::Renderer(int width, int height) :
	m_width(width),
	m_height(height),
	m_resolveFBO(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}},
				 DepthMode::Depth),
	m_dirShadowDepthFBO(2048, 2048, 
		{}, DepthMode::DepthTexture),
	m_brightFBO(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}}),
	m_pingpongFBO{
		Framebuffer(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}}),
		Framebuffer(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}})},
	m_lightShader("shaders/lightobj.vs", "shaders/lightobj.fs"),
	m_brightPassShader("shaders/framebuffer.vs", "shaders/bloomBrightPass.fs"),
	m_blurShader("shaders/framebuffer.vs", "shaders/bloomBlur.fs"),
	m_postFXShader("shaders/framebuffer.vs", "shaders/framebuffer.fs"),
	m_debugShader("shaders/debugshader1.vs", "shaders/debugshader1.fs") {
	initQuad();
}
 
void Renderer::resize(int width, int height) {
	m_width = width;
	m_height = height;
	m_resolveFBO.resize(width, height);
	m_brightFBO.resize(width, height);
	m_pingpongFBO[0].resize(width, height);
	m_pingpongFBO[1].resize(width, height);
}

void Renderer::render(Scene &scene) {
	renderForward(scene);
	//if (m_bloomEnabled)
	//	bloom();
	//postFX();
}

void Renderer::renderForward(Scene &scene) {
	glViewport(0, 0, m_width, m_height);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto &cam = scene.getCamera();
	glm::mat4 view = cam.getViewMatrix();
	glm::mat4 projection =
		cam.getProjectionMatrix((float)m_width / (float)m_height);

	m_debugShader.use();
	m_debugShader.setMat4("view", view);
	m_debugShader.setMat4("projection", projection);
	m_debugShader.setVec3("viewPos", cam.Position);
	m_debugShader.setInt("material.texture_diffuse", TextureUnit::Diffuse);
	m_debugShader.setInt("material.texture_specular", TextureUnit::Specular);
	m_debugShader.setBool("blinn", true);

	setLightUniforms(scene);

	for (auto &obj : scene.getObjects()) {
		if (!obj.active || !obj.mesh)
			continue;
		m_debugShader.setMat4("model", obj.getModelMatrix());
		if (obj.material) {
			bindMaterial(*obj.material);
			// std::cout << obj.material->spec << std::endl;
		}
		obj.mesh->draw();
	}
}

void Renderer::bindMaterial(const Material &mat) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mat.diff);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mat.spec);
	//std::cout << mat.spec << std::endl;
	/*
	m_lightShader.setBool("isNormalMapped", mat.isNormalMapped);
	if (mat.isNormalMapped) {
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, mat.norm);
	}

	m_lightShader.setBool("isParallaxMapped", mat.isParallaxMapped);
	if (mat.isParallaxMapped) {
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, mat.parallaxMap);
		m_lightShader.setFloat("height_scale", mat.heightScale);
	}
	*/
	m_debugShader.setFloat("material.shininess", mat.shininess);
}

void Renderer::setLightUniforms(Scene &scene) {
	auto &dl = scene.getDirLight();
	m_debugShader.setVec3("dirlight.direction", dl.direction);
	m_debugShader.setVec3("dirlight.ambient", dl.ambient);
	m_debugShader.setVec3("dirlight.diffuse", dl.diffuse);
	m_debugShader.setVec3("dirlight.specular", dl.specular);

	auto &lights = scene.getPointLights();
	m_debugShader.setInt("pl_num", (int)scene.getPointLights().size());
	for (size_t i = 0; i < lights.size(); i++) {
		std::string p = "pLights[" + std::to_string(i) + "].";
		m_debugShader.setFloat(p + "constant", lights[i].constant);
		m_debugShader.setFloat(p + "linear", lights[i].linear);
		m_debugShader.setFloat(p + "quadratic", lights[i].quadratic);
		m_debugShader.setVec3(p + "ambient", lights[i].ambient);
		m_debugShader.setVec3(p + "diffuse", lights[i].diffuse);
		m_debugShader.setVec3(p + "specular", lights[i].specular);
		m_debugShader.setVec3(p + "position", lights[i].position);
	}

	auto &sl = scene.getSpotLight();
	m_debugShader.setInt("sl_num", (int)scene.getSpotLight().size());
	for (size_t i = 0; i < sl.size(); i++) {
		std::string p = "spotlight[" + std::to_string(i) + "].";
		m_debugShader.setVec3(p + "position", sl[i].position);
		m_debugShader.setVec3(p + "direction", sl[i].direction);
		m_debugShader.setVec3(p + "ambient", sl[i].ambient);
		m_debugShader.setVec3(p + "diffuse", sl[i].diffuse);
		m_debugShader.setVec3(p + "specular", sl[i].specular);
		m_debugShader.setFloat(p + "cutoff", sl[i].cutoff);
		m_debugShader.setFloat(p + "outercutoff", sl[i].outerCutoff);
		m_debugShader.setFloat(p + "constant", sl[i].constant);
		m_debugShader.setFloat(p + "linear", sl[i].linear);
		m_debugShader.setFloat(p + "quadratic", sl[i].quadratic);
	}
}
 
void Renderer::bloom() {
	m_brightFBO.bind();
	glClear(GL_COLOR_BUFFER_BIT);
	m_brightPassShader.use();
	m_brightPassShader.setInt("sceneTex", 0);
	m_brightPassShader.setFloat("threshold", 1.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_resolveFBO.getColorAttachment(0));
	drawQuad();

	bool horizontal = true, first = true;
	int amount = 10;
	m_blurShader.use();
	for (int i = 0; i < amount; i++) {
		m_pingpongFBO[horizontal].bind();
		m_blurShader.setBool("horizontal", horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,
					  first ? m_brightFBO.getColorAttachment(0)
							: m_pingpongFBO[!horizontal].getColorAttachment(0));
		drawQuad();
		horizontal = !horizontal;
		first = false;
	}
}

void Renderer::postFX() {
	Framebuffer::bindDefault();
	glViewport(0, 0, m_width, m_height);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	m_postFXShader.use();
	m_postFXShader.setInt("ourTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_resolveFBO.getColorAttachment(0));

	m_postFXShader.setInt("bloomTex", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pingpongFBO[0].getColorAttachment(
									 0)); // last-written buffer

	m_postFXShader.setBool("hdr", m_hdrEnabled);
	m_postFXShader.setFloat("exposure", m_exposure);
	m_postFXShader.setInt("fbmode", m_postFXMode);
	m_postFXShader.setBool("bloom", m_bloomEnabled);
	drawQuad();
}

void Renderer::initQuad() {
	float quadVerts[] = {-1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
						 1.0f,	-1.0f, 1.0f, 0.0f, -1.0f, 1.0f,	 0.0f, 1.0f,
						 1.0f,	-1.0f, 1.0f, 0.0f, 1.0f,  1.0f,	 1.0f, 1.0f};
	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &m_quadVBO);
	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
						  (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
						  (void *)(2 * sizeof(float)));
	glBindVertexArray(0);
}

void Renderer::drawQuad() {
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}