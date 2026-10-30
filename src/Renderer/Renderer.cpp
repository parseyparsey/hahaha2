#include "Renderer.h"

Renderer::Renderer(int width, int height) :
	m_width(width),
	m_height(height),
	m_skyboxMesh(h32rf::makeSkyboxVertices(), std::vector<unsigned int>{}),
	m_resolveFBO(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}},
				 DepthMode::Depth),
	m_dirShadowDepthFBO(m_dirShadowMapSize, m_dirShadowMapSize, 
		std::vector<AttachmentSpec>{}, DepthMode::DepthTexture),
	m_brightFBO(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}}),
	m_pingpongFBO{
		Framebuffer(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}}),
		Framebuffer(width, height, {{GL_RGBA16F, GL_RGBA, GL_FLOAT}})},
	m_ldrFBO(width, height, {{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}}),
	m_lightShader("shaders/lightobj.vs", "shaders/lightobj.fs"),
	m_brightPassShader("shaders/h32_framebuffer.vs", "shaders/h32_bloomBrightPass.fs"),
	m_blurShader("shaders/h32_framebuffer.vs", "shaders/h32_bloomBlur.fs"),
	m_postFXShader("shaders/h32_framebuffer.vs", "shaders/h32_framebuffer.fs"),
	m_debugShader("shaders/h32_debugshader1.vs", "shaders/h32_debugshader21.fs"),
	m_dirShadowDepthShader("shaders/h32_depth_shader.vs", "shaders/h32_depth_shader.fs"),
	m_skyboxShader("shaders/h32_skybox.vs", "shaders/h32_skybox.fs"),
	m_unlitShader("shaders/h32_unlit.vs", "shaders/h32_unlit.fs"),
	m_fxaaShader("shaders/framebuffer.vs", "shaders/h32_fxaa.fs"),
	m_pointShadowDepthShader("shaders/h32_pshadow_depth.vs",
							 "shaders/h32_pshadow_depth.fs","shaders/h32_pshadow_depth.gs")
{
	initQuad();
	h32rf::whiteTxt(m_whiteTex);
	
	for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
		m_spotShadowDepthFBO.emplace_back(m_spotShadowMapSize, m_spotShadowMapSize,
									 std::vector<AttachmentSpec>{},
									 DepthMode::DepthTexture);

	for (int i = 0; i < MAX_POINT_SHADOW; i++)
		m_pointShadowFBO.emplace_back(
			m_pointShadowMapSize, m_pointShadowMapSize,
			std::vector<AttachmentSpec>{}, DepthMode::DepthCubemap);
}

void Renderer::resize(int width, int height) {
	m_width = width;
	m_height = height;
	m_resolveFBO.resize(width, height);
	m_brightFBO.resize(width, height);
	m_pingpongFBO[0].resize(width, height);
	m_pingpongFBO[1].resize(width, height);
	m_ldrFBO.resize(width, height);
}

void Renderer::render(Scene &scene) {
	renderDirShadow(scene);
	renderPointShadow(scene);
	renderSpotShadow(scene);
	renderForward(scene);
	if (m_bloomEnabled)
		bloom();
	postFX();
}

void Renderer::renderForward(Scene &scene) {
	glViewport(0, 0, m_width, m_height);

	m_resolveFBO.bind();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto &cam = scene.getCamera();
	glm::mat4 view = cam.getViewMatrix();
	glm::mat4 projection =
		cam.getProjectionMatrix((float)m_width / (float)m_height);

	m_debugShader.use();

	glActiveTexture(GL_TEXTURE0 + TextureUnit::DirShadowMap);
	glBindTexture(GL_TEXTURE_2D, m_dirShadowDepthFBO.getDepthAttachment());

	m_debugShader.setMat4("view", view);
	m_debugShader.setMat4("projection", projection);
	m_debugShader.setVec3("viewPos", cam.Position);
	m_debugShader.setInt("material.texture_diffuse", TextureUnit::Diffuse);
	m_debugShader.setInt("dirShadowMap", TextureUnit::DirShadowMap);
	m_debugShader.setMat4("LightSpaceMatrix", lightSpaceMatrix);
	m_debugShader.setBool("blinn", true);

	m_debugShader.setInt("normalMap", TextureUnit::NormalMap);
	m_debugShader.setInt("parallaxDepthMap", TextureUnit::ParallaxMap);

	for (int i = 0; i < (int)m_spotLightSpaceMatrices.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + TextureUnit::SpotShadowBase + i);
		glBindTexture(GL_TEXTURE_2D, m_spotShadowDepthFBO[i].getDepthAttachment());
		m_debugShader.setInt("spotShadowMap[" + std::to_string(i) + "]",
							 TextureUnit::SpotShadowBase + i);
		m_debugShader.setMat4("spotLightSpace[" + std::to_string(i) + "]",
							  m_spotLightSpaceMatrices[i]); 
	}

	for (int i = 0; i < MAX_POINT_SHADOW; i++) {
		m_debugShader.setInt("pointShadowMap[" + std::to_string(i) + "]", 
			TextureUnit::PointShadowMap + i);
		if (i < m_pointShadowCount) {
			glActiveTexture(GL_TEXTURE0 + TextureUnit::PointShadowMap + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_pointShadowFBO[i].getDepthAttachment());
		}
	}
	m_debugShader.setInt("pointShadowCount", m_pointShadowCount);
	m_debugShader.setFloat("pointFarPlane", m_pointFarPlane);
	
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

	renderSkybox(scene);

	m_unlitShader.use();
	m_unlitShader.setMat4("view", view);
	m_unlitShader.setMat4("projection", projection);
	for (auto &l : scene.getPointLights()) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), l.position);
		model = glm::scale(model, glm::vec3(0.1f));
		m_unlitShader.setMat4("model", model);
		m_unlitShader.setVec3(
			"color", l.diffuse * 4.0f);
		m_skyboxMesh.draw();
	}
}

void Renderer::bindMaterial(const Material &mat) {
	
	bool n = mat.isNormalMapped(), p = mat.isParallaxMapped();
	m_debugShader.setBool("useNormalMap", n);
	if (n) {
		glActiveTexture(GL_TEXTURE0 + TextureUnit::NormalMap);
		glBindTexture(GL_TEXTURE_2D, mat.norm);
	}

	m_debugShader.setBool("useParallaxMap", p);
	m_debugShader.setBool("parallaxClip", mat.parallaxClip);
	if (p) {
		glActiveTexture(GL_TEXTURE0 + TextureUnit::ParallaxMap);
		glBindTexture(GL_TEXTURE_2D, mat.parallaxMap);
		m_debugShader.setFloat("height_scale", mat.heightScale);
	}

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
	m_debugShader.setInt("shadingModel", mat.model);

	auto pick = [&](unsigned int tex) { return tex ? tex : m_whiteTex; };

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pick(mat.diff));

	if (mat.model == ShadingModel::BLINNPHONG) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mat.spec);
		m_debugShader.setInt("material.texture_specular",TextureUnit::Specular);
		m_debugShader.setFloat("material.shininess", mat.shininess);
	} else if (mat.model == ShadingModel::PBR) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pick(mat.metallicMap));
		m_debugShader.setInt("pbrMat.metallicMap",TextureUnit::Metallic);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pick(mat.roughnessMap));
		m_debugShader.setInt("pbrMat.roughnessMap", TextureUnit::Roughness);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pick(mat.aoMap));
		m_debugShader.setInt("pbrMat.aoMap", TextureUnit::AO);
	}
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_brightPassShader.use();
	m_brightPassShader.setInt("sceneTex", 0);
	m_brightPassShader.setFloat("threshold", 2.0f);
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
	if (m_fxaaEnabled)
		m_ldrFBO.bind();
	else
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

	if (m_fxaaEnabled)
	{
		Framebuffer::bindDefault();
		glViewport(0, 0, m_width, m_height);
		glClear(GL_COLOR_BUFFER_BIT);
		m_fxaaShader.use();
		m_fxaaShader.setInt("screenTex", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ldrFBO.getColorAttachment(0));
		drawQuad();
	}
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

void Renderer::renderDirShadow(Scene &scene) {
	auto &obj = scene.getDirLight();

	float near_plane = 1.0f, far_plane = 200.0f; // 7.5f;
	glm::mat4 lightProjection =
		glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);
		//glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
	glm::vec3 lightDir = glm::normalize(obj.direction);
	glm::mat4 lightView = glm::lookAt(-lightDir * 20.0f, glm::vec3(0.0f),
									  glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;

	m_dirShadowDepthShader.use();
	m_dirShadowDepthShader.setMat4("LightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, m_dirShadowMapSize, m_dirShadowMapSize);
	m_dirShadowDepthFBO.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	for (auto &obj : scene.getObjects()) {
		if (!obj.active || !obj.mesh)
			continue;
		m_dirShadowDepthShader.setMat4("model", obj.getModelMatrix());
		obj.mesh->draw();
	}

	Framebuffer::bindDefault();
	glViewport(0, 0, m_width, m_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	static bool printed = false;
	if (!printed) {
		for (int r = 0; r < 4; r++)
			std::cout << lightSpaceMatrix[0][r] << " " << lightSpaceMatrix[1][r]
					  << " " << lightSpaceMatrix[2][r] << " "
					  << lightSpaceMatrix[3][r] << "\n";
		std::cout << "dir: " << obj.direction.x << " " << obj.direction.y << " "
				  << obj.direction.z << "\n";
		printed = true;
	}*/
}

glm::mat4 Renderer::spotLightSpaceMatrix(SpotLight& sl) {
	float outerAngle = glm::degrees(glm::acos(sl.outerCutoff));
	float fov = outerAngle * 2.0f + 2.0f;
	glm::mat4 proj = glm::perspective(glm::radians(fov), 1.0f, 0.1f, 50.0f);

	glm::vec3 dir = glm::normalize(sl.direction);
	glm::vec3 up = std::abs(dir.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
	glm::mat4 view = glm::lookAt(sl.position, sl.position + dir, up);
	return proj * view;
}

void Renderer::renderSpotShadow(Scene &scene) {
	auto &sl = scene.getSpotLight();

	int sl_count = std::min((int)sl.size(), MAX_SPOT_LIGHTS);
	m_spotLightSpaceMatrices.assign(sl_count, glm::mat4(1.0f));

	m_dirShadowDepthShader.use();
	glViewport(0, 0, m_spotShadowMapSize, m_spotShadowMapSize);
	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < sl_count; i++) {
		m_spotLightSpaceMatrices[i] = spotLightSpaceMatrix(sl[i]);
		m_dirShadowDepthShader.setMat4("LightSpaceMatrix", m_spotLightSpaceMatrices[i]);

		m_spotShadowDepthFBO[i].bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		for (auto &obj : scene.getObjects()) {
			if (!obj.active || !obj.mesh)
				continue;
			m_dirShadowDepthShader.setMat4("model", obj.getModelMatrix());
			obj.mesh->draw();
		}
	}

	Framebuffer::bindDefault();
	glViewport(0, 0, m_width, m_height);
}

void Renderer::renderSkybox(Scene &scene) {
	unsigned int cubemap = scene.getSkybox().cubemap;

	if (!cubemap)
		return;

	auto &cam = scene.getCamera();

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	m_skyboxShader.use();
	m_skyboxShader.setMat4(
		"view", glm::mat4(glm::mat3(cam.getViewMatrix())));
	m_skyboxShader.setMat4("projection",
						   cam.getProjectionMatrix((float)m_width / m_height));

	glActiveTexture(GL_TEXTURE0 + TextureUnit::SkyboxMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	m_skyboxShader.setInt("skybox", TextureUnit::SkyboxMap);
	m_skyboxMesh.draw();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Renderer::renderPointShadow(Scene &scene) {
	auto &lights = scene.getPointLights();
	int n = std::min((int)lights.size(), MAX_POINT_SHADOW);

	const float nearplane = 0.1f, farplane = m_pointFarPlane;
	glm::mat4 proj =
		glm::perspective(glm::radians(90.0f), 1.0f, nearplane, farplane);

	m_pointShadowDepthShader.use();
	glViewport(0, 0, m_pointShadowMapSize, m_pointShadowMapSize);
	glEnable(GL_DEPTH_TEST); 

	for (int i = 0; i < n; i++) {
		glm::vec3 p = lights[i].position;
		glm::mat4 t[6] = {
			proj * glm::lookAt(p, p + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
			proj * glm::lookAt(p, p + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
			proj * glm::lookAt(p, p + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
			proj * glm::lookAt(p, p + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
			proj * glm::lookAt(p, p + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
			proj * glm::lookAt(p, p + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
		};

		for (int k = 0; k < 6; k++)
			m_pointShadowDepthShader.setMat4(
				"shadowMatrices[" + std::to_string(k) + "]", t[k]);

		m_pointShadowDepthShader.setVec3("lightPos", p);
		m_pointShadowDepthShader.setFloat("far_plane", farplane);

		m_pointShadowFBO[i].bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		for (auto &obj : scene.getObjects()) {
			if (!obj.active || !obj.mesh)
				continue;
			m_pointShadowDepthShader.setMat4("model", obj.getModelMatrix());
			obj.mesh->draw();
		}
	}

	m_pointShadowCount = n;
	Framebuffer::bindDefault();
	glViewport(0, 0, m_width, m_height);
}