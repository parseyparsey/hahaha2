#include "Renderer.h"

Renderer::Renderer(int width, int height) :
	m_width(width),
	m_height(height),
	m_resolveFBO(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}},
				 DepthMode::Depth),
	m_brightFBO(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}}),
	m_pingpongFBO{
		Framebuffer(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}}),
		Framebuffer(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}})},
	m_lightShader("shaders/lightobj.vs", "shaders/lightobj.fs"),
	m_brightPassShader("shaders/framebuffer.vs", "shaders/bloomBrightPass.fs"),
	m_blurShader("shaders/framebuffer.vs", "shaders/bloomBlur.fs"),
	m_postFXShader("shaders/framebuffer.vs", "shaders/framebuffer.fs") {
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
	if (m_bloomEnabled)
		bloom();
	postFX();
}

void Renderer::renderForward(Scene &scene) {
	m_resolveFBO.bind();
	glViewport(0, 0, m_width, m_height);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto &cam = scene.getCamera();
	glm::mat4 view = cam.getViewMatrix();
	glm::mat4 projection =
		cam.getProjectionMatrix((float)m_width / (float)m_height);

	m_lightShader.use();
	m_lightShader.setMat4("view", view);
	m_lightShader.setMat4("projection", projection);
	m_lightShader.setVec3("viewPos", cam.Position);
	m_lightShader.setInt("material.texture_diffuse", 0);
	m_lightShader.setInt("material.texture_specular", 1);
	m_lightShader.setInt("normalMap", 7);
	m_lightShader.setInt("parallaxDepthMap", 8);

	setLightUniforms(scene);

	for (auto &obj : scene.getObjects()) {
		if (!obj.active || !obj.mesh)
			continue;
		m_lightShader.setMat4("model", obj.getModelMatrix());
		if (obj.material)
			bindMaterial(*obj.material);
		obj.mesh->draw();
	}
}

void Renderer::bindMaterial(const Material &mat) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mat.diff);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mat.spec);

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

	m_lightShader.setFloat("material.shininess", mat.shininess);
}

void Renderer::setLightUniforms(Scene &scene) {
	auto &dl = scene.getDirLight();
	m_lightShader.setVec3("dirlight.direction", dl.direction);
	m_lightShader.setVec3("dirlight.ambient", dl.ambient);
	m_lightShader.setVec3("dirlight.diffuse", dl.diffuse);
	m_lightShader.setVec3("dirlight.specular", dl.specular);

	auto &lights = scene.getPointLights();
	for (size_t i = 0; i < lights.size(); i++) {
		std::string p = "pLight[" + std::to_string(i) + "].";
		m_lightShader.setFloat(p + "constant", lights[i].constant);
		m_lightShader.setFloat(p + "linear", lights[i].linear);
		m_lightShader.setFloat(p + "quadratic", lights[i].quadratic);
		m_lightShader.setVec3(p + "ambient", lights[i].ambient);
		m_lightShader.setVec3(p + "diffuse", lights[i].diffuse);
		m_lightShader.setVec3(p + "specular", lights[i].specular);
		m_lightShader.setVec3(p + "position", lights[i].position);
	}

	auto &sl = scene.getSpotLight();
	m_lightShader.setVec3("spotlight.position", sl.position);
	m_lightShader.setVec3("spotlight.direction", sl.direction);
	m_lightShader.setVec3("spotlight.ambient", sl.ambient);
	m_lightShader.setVec3("spotlight.diffuse", sl.diffuse);
	m_lightShader.setVec3("spotlight.specular", sl.specular);
	m_lightShader.setFloat("spotlight.cutoff", sl.cutoff);
	m_lightShader.setFloat("spotlight.outercutoff", sl.outerCutoff);
	m_lightShader.setFloat("spotlight.constant", sl.constant);
	m_lightShader.setFloat("spotlight.linear", sl.linear);
	m_lightShader.setFloat("spotlight.quadratic", sl.quadratic);
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