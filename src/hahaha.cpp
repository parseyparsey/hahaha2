//============================================================================
// Name        : hahaha.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <Core/Window.h>
#include "Renderer/Renderer.h"
#include "Texture.h"
#include "Scene/Scene.h"
#include "Input/Input.h"

Window g_window(800, 600, "hahaha2");
Scene scene;
Renderer renderer(800, 600);

Texture containerDiffuse("textures/container2.png", false,
						 false);
Texture containerSpecular("textures/container2_specular.png", false, false);
Texture whitetxt("textures/white.png", false, false);
float dt;
float lasttime;

void confScene();
void updateMyScene();
std::vector<Vertex> makeCubeVertices();

int main() { 
	g_window.onResizeCallback = [&](int w, int h) { renderer.resize(w, h); };

	g_window.onMouseMoveCallback = [&](float xOffset, float yOffset) {
		scene.getCamera().ProcessMouseMovement(xOffset, yOffset);
	};

	glfwSetInputMode(g_window.getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	std::cout << containerDiffuse.ID << std::endl;

	confScene();

	while (!g_window.shouldClose()) {
		float currentFrame = glfwGetTime();
		dt = currentFrame - lasttime;
		lasttime = currentFrame;
		
		g_window.pollEvents();
		processInput(g_window.getHandle(), scene, dt);
		updateMyScene();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.render(scene);
		g_window.swapBuffers();
	}
}

void confScene() { 
	Mesh &cubeMesh = scene.addMesh(makeCubeVertices());

	Material placeholderMat{};
	placeholderMat.diff = containerDiffuse.ID;
	placeholderMat.spec = containerSpecular.ID;
	Material &container_mat = scene.addMaterial(placeholderMat);

	glm::vec3 cubePositions[] = {
		glm::vec3(2.0f, 2.0f, -4.0f),	glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),	glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),	glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),	glm::vec3(-1.3f, 1.0f, -1.5f),
		glm::vec3(-6.0f, -2.0f, -22.0f)};

	int cubenum = sizeof(cubePositions) / sizeof(cubePositions[0]);

	for (unsigned int i = 0; i < cubenum; i++) {
		const char* cubename = ("cube" + std::to_string(i)).c_str();
		GameObject &cube = scene.addObject(cubename, &cubeMesh, &container_mat);
		cube.transform.position = cubePositions[i];
		//float angle = 20.0f * i;
	}

	auto &dl = scene.getDirLight();
	dl.direction = glm::vec3(0.0f, -1.0f, -0.3f);
	dl.ambient = glm::vec3(0.01f);
	dl.diffuse = glm::vec3(0.2f);
	dl.specular = glm::vec3(0.0f);

	
	glm::vec3 plightPos[] = {
		glm::vec3(0.0f, 0.0f, -3.0f),	glm::vec3(5.0f, 5.0f, -5.0f),
		glm::vec3(-5.0f, 2.0f, -5.0f),	glm::vec3(0.0f, -2.0f, -10.0f),
		glm::vec3(3.0f, -2.0f, -10.0f), glm::vec3(2.0f, 1.5f, -5.0f),
	};

	auto &pl = scene.getPointLights();

	PointLight pl0;
	pl0.position = plightPos[0];
	pl0.constant = 1.0f;
	pl0.linear = 0.09f;
	pl0.quadratic = 0.032f;
	pl0.ambient = glm::vec3(0.8f);
	pl0.diffuse = glm::vec3(0.8f);
	pl0.specular = glm::vec3(0.3f);
	pl.push_back(pl0);

	placeholderMat.diff = whitetxt.ID;
	placeholderMat.spec = whitetxt.ID;
	Material &plight_mat = scene.addMaterial(placeholderMat);
	
	GameObject &cubelight = scene.addObject("pl0", &cubeMesh, &plight_mat);
	cubelight.transform.position = plightPos[0];
	cubelight.transform.scale = glm::vec3(0.3);

	SpotLight sl0{
		glm::vec3(0.0f), glm::vec3(0.0f), 
		glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 
		glm::cos(glm::radians(12.5f)),
		glm::cos(glm::radians(17.5f)),
		1.0f, 0.45f, 0.0075f
	};

	auto &sl = scene.getSpotLight();
	sl.push_back(sl0);
	

	/* scene.getPointLights().push_back({
		glm::vec3(1.0f, 2.0f, -3.0f), // position
		1.0f, 0.09f, 0.032f,		  // constant, linear, quadratic
		glm::vec3(0.05f),			  // ambient
		glm::vec3(0.6f),			  // diffuse
		glm::vec3(0.3f)				  // specular
	});*/
}

void updateMyScene() { 
	auto &sl = scene.getSpotLight(); 
	sl.front().position = scene.getCamera().Position;
	sl.front().direction = scene.getCamera().Front;

	if (flashlight_state == 1) {
		sl.front().ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		sl.front().diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		sl.front().specular = glm::vec3(1.0f, 1.0f, 1.0f);
	} else if (flashlight_state == 2) {
		sl.front().ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		sl.front().diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
		sl.front().specular = glm::vec3(0.5f, 0.5f, 0.5f);
	} else if (flashlight_state == 0) {
		sl.front().ambient = glm::vec3(0.0f, 0.0f, 0.0f);
		sl.front().diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
		sl.front().specular = glm::vec3(0.0f, 0.0f, 0.0f);
	}
}

std::vector<Vertex> makeCubeVertices() {
	// position, normal, texcoord — tangent/bitangent zeroed (unused without
	// normal mapping)
	float raw[] = {
		// positions          // normals           // texcoords
		-0.5f, -0.5f, -0.5f, 0.0f,	0.0f,  -1.0f, 0.0f,	 0.0f,	0.5f,  -0.5f,
		-0.5f, 0.0f,  0.0f,	 -1.0f, 1.0f,  0.0f,  0.5f,	 0.5f,	-0.5f, 0.0f,
		0.0f,  -1.0f, 1.0f,	 1.0f,	0.5f,  0.5f,  -0.5f, 0.0f,	0.0f,  -1.0f,
		1.0f,  1.0f,  -0.5f, 0.5f,	-0.5f, 0.0f,  0.0f,	 -1.0f, 0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,	0.0f,  -1.0f, 0.0f,	 0.0f,

		-0.5f, -0.5f, 0.5f,	 0.0f,	0.0f,  1.0f,  0.0f,	 0.0f,	0.5f,  -0.5f,
		0.5f,  0.0f,  0.0f,	 1.0f,	1.0f,  0.0f,  0.5f,	 0.5f,	0.5f,  0.0f,
		0.0f,  1.0f,  1.0f,	 1.0f,	0.5f,  0.5f,  0.5f,	 0.0f,	0.0f,  1.0f,
		1.0f,  1.0f,  -0.5f, 0.5f,	0.5f,  0.0f,  0.0f,	 1.0f,	0.0f,  1.0f,
		-0.5f, -0.5f, 0.5f,	 0.0f,	0.0f,  1.0f,  0.0f,	 0.0f,

		-0.5f, 0.5f,  0.5f,	 -1.0f, 0.0f,  0.0f,  1.0f,	 0.0f,	-0.5f, 0.5f,
		-0.5f, -1.0f, 0.0f,	 0.0f,	1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
		0.0f,  0.0f,  0.0f,	 1.0f,	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
		0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,	 0.0f,	0.0f,  0.0f,
		-0.5f, 0.5f,  0.5f,	 -1.0f, 0.0f,  0.0f,  1.0f,	 0.0f,

		0.5f,  0.5f,  0.5f,	 1.0f,	0.0f,  0.0f,  1.0f,	 0.0f,	0.5f,  0.5f,
		-0.5f, 1.0f,  0.0f,	 0.0f,	1.0f,  1.0f,  0.5f,	 -0.5f, -0.5f, 1.0f,
		0.0f,  0.0f,  0.0f,	 1.0f,	0.5f,  -0.5f, -0.5f, 1.0f,	0.0f,  0.0f,
		0.0f,  1.0f,  0.5f,	 -0.5f, 0.5f,  1.0f,  0.0f,	 0.0f,	0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,	 1.0f,	0.0f,  0.0f,  1.0f,	 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f,	-1.0f, 0.0f,  0.0f,	 1.0f,	0.5f,  -0.5f,
		-0.5f, 0.0f,  -1.0f, 0.0f,	1.0f,  1.0f,  0.5f,	 -0.5f, 0.5f,  0.0f,
		-1.0f, 0.0f,  1.0f,	 0.0f,	0.5f,  -0.5f, 0.5f,	 0.0f,	-1.0f, 0.0f,
		1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,	0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,	-1.0f, 0.0f,  0.0f,	 1.0f,

		-0.5f, 0.5f,  -0.5f, 0.0f,	1.0f,  0.0f,  0.0f,	 1.0f,	0.5f,  0.5f,
		-0.5f, 0.0f,  1.0f,	 0.0f,	1.0f,  1.0f,  0.5f,	 0.5f,	0.5f,  0.0f,
		1.0f,  0.0f,  1.0f,	 0.0f,	0.5f,  0.5f,  0.5f,	 0.0f,	1.0f,  0.0f,
		1.0f,  0.0f,  -0.5f, 0.5f,	0.5f,  0.0f,  1.0f,	 0.0f,	0.0f,  0.0f,
		-0.5f, 0.5f,  -0.5f, 0.0f,	1.0f,  0.0f,  0.0f,	 1.0f};

	std::vector<Vertex> verts;
	for (int i = 0; i < 36; i++) {
		Vertex v{};
		v.position = glm::vec3(raw[i * 8 + 0], raw[i * 8 + 1], raw[i * 8 + 2]);
		v.normal = glm::vec3(raw[i * 8 + 3], raw[i * 8 + 4], raw[i * 8 + 5]);
		v.texCoords = glm::vec2(raw[i * 8 + 6], raw[i * 8 + 7]);
		v.tangent = glm::vec3(0.0f);
		v.bitangent = glm::vec3(0.0f);
		verts.push_back(v);
	}
	return verts;
}