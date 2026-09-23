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
#include "Scene/Scene.h"
#include "Input/Input.h"

Window g_window(800, 600, "h32engine");
Scene scene;
Renderer renderer(800, 600);

float dt;
float lasttime;

void confScene();
std::vector<Vertex> makeCubeVertices();

int main() { 
	g_window.onResizeCallback = [&](int w, int h) { renderer.resize(w, h); };

	g_window.onMouseMoveCallback = [&](float xOffset, float yOffset) {
		scene.getCamera().ProcessMouseMovement(xOffset, yOffset);
	};

	while (!g_window.shouldClose()) {
		float currentFrame = glfwGetTime();
		dt = currentFrame - lasttime;
		lasttime = currentFrame;
		
		g_window.pollEvents();
		processInput(g_window.getHandle(), scene, dt);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.render(scene);
		g_window.swapBuffers();
	}
}

void confScene() { 
	Mesh &cubeMesh = scene.addMesh(makeCubeVertices());

	Material placeholderMat{};
	placeholderMat.diff =
		0; // 0 = no texture bound; fine for a first smoke test
	placeholderMat.spec = 0;
	Material &mat = scene.addMaterial(placeholderMat);

	GameObject &cube = scene.addObject("TestCube", &cubeMesh, &mat);
	cube.transform.position = glm::vec3(0.0f, 0.0f, -3.0f);

	auto &dl = scene.getDirLight();
	dl.direction = glm::vec3(0.0f, -1.0f, -0.3f);
	dl.ambient = glm::vec3(0.2f);
	dl.diffuse = glm::vec3(0.8f);
	dl.specular = glm::vec3(0.5f);
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