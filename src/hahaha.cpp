//============================================================================
// Name        : hahaha.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <Core/Window.h>
#include <vector>
#include "Renderer/Renderer.h"
#include "texture.h" 
#include "Scene/Scene.h"
#include "Input/Input.h"
#include "Assets/ModelLoader.h"
#include "Assets/TextureCache.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "plane.h"

Window g_window(800, 600, "hahaha2");
Renderer renderer(800, 600);
TextureCache textures;
Scene scene;

Texture containerDiffuse("textures/container2.png", false,
						 true);
Texture containerSpecular("textures/container2_specular.png", false, false);
Texture whitetxt("textures/white.png", false, false);
Texture planetxt("textures/wood.png", false, true);
Texture naotxt("textures/nao_cropped.jpg", true, true, 1);
Texture naotxt1("textures/nao08.jpg", true, true, 1);
Texture bricks2_diff(/*"textures/bricks2.jpg"*/"textures/toy_box_diffuse.png", false, true);
Texture bricks2_norm(
	/*"textures/bricks2_normal.jpg"*/ "textures/toy_box_normal.png", false,
	false); 
Texture
	bricks2_disp(/*"textures/bricks2_disp.jpg"*/ "textures/toy_box_disp.png",
				 false, false);
Texture brickwall_diff("textures/brickwall.jpg", false, true);
Texture brickwall_norm("textures/brickwall_normal.jpg", false, false);

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
	Mesh &cubeMesh = scene.addMesh(h32rf::makeCubeVertices());

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
	}

	placeholderMat.diff = bricks2_diff.ID;
	placeholderMat.spec = whitetxt.ID;
	placeholderMat.norm = bricks2_norm.ID;
	placeholderMat.parallaxMap = bricks2_disp.ID;
	placeholderMat.parallaxClip = true;
	Material &POM_mat = scene.addMaterial(placeholderMat);

	auto vertscubePOM = h32rf::makeCubeVertices();
	h32rf::computeTangents(vertscubePOM);
	Mesh &cubeMeshPOM = scene.addMesh(vertscubePOM);
	GameObject &cubePOM = scene.addObject("cubePOM", &cubeMeshPOM, &POM_mat);
	cubePOM.transform.position = glm::vec3(3.0f, 0.0f, -6.0f);
	cubePOM.transform.rotation = glm::vec3(0.0f, -45.0f, 0.0f);

	placeholderMat.parallaxMap = 0;
	placeholderMat.parallaxClip = false;

	placeholderMat.diff = brickwall_diff.ID;
	placeholderMat.norm = brickwall_norm.ID; 
	Material &CubeN_mat = scene.addMaterial(placeholderMat);

	GameObject &cubeN = scene.addObject("cubeN", &cubeMeshPOM, &CubeN_mat);
	cubeN.transform.position = glm::vec3(-3.0f, 0.0f, -6.0f);
	cubeN.transform.rotation = glm::vec3(0.0f, 45.0f, 0.0f);

	placeholderMat.diff = naotxt.ID;
	placeholderMat.spec = 0;
	placeholderMat.norm = 0;

	Material &naomat = scene.addMaterial(placeholderMat);
	GameObject &naocube = scene.addObject("naocube", &cubeMesh, &naomat);
	naocube.transform.position = glm::vec3(0.0f, 0.0f, -10.0f);

	placeholderMat.diff = naotxt1.ID;
	placeholderMat.spec = 0;
	Material &naomat1 = scene.addMaterial(placeholderMat);
	GameObject &naocube1 = scene.addObject("naocube1", &cubeMesh, &naomat1);
	naocube1.transform.position = glm::vec3(5.0f, 2.0f, -8.0f);
	naocube1.transform.scale = glm::vec3(1.216f, 1.0f, 1.0f);
	naocube1.transform.rotation = glm::vec3(0.0f, -45.0f, 0.0f);

	ModelData backpackData;
	ModelLoader::load("models/backpack/backpack.obj", backpackData);
	auto backpack =
		ModelLoader::instantiate(scene, backpackData, textures, "backpack", {false, true, false});

	for (size_t i : backpack) {
		auto &t = scene.getObjects()[i].transform;
		t.position = glm::vec3(-6.0f, 1.0f, -6.0f);
		t.scale = glm::vec3(0.6f, 0.6f, 0.6f);
	}

	ModelData cyborgData;
	ModelLoader::load("models/cyborg/cyborg.obj", cyborgData);
	auto cyborg = ModelLoader::instantiate(scene, cyborgData, textures,
											 "cyborg", {true, true, true});

	for (size_t i : cyborg) {
		auto &t = scene.getObjects()[i].transform;
		t.position = glm::vec3(-3.0f, 3.0f, -6.0f);
		t.scale = glm::vec3(0.6f, 0.6f, 0.6f);
	}

	Plane plane(150);
	placeholderMat.diff = planetxt.ID;
	placeholderMat.spec = whitetxt.ID;
	Material &plane_mat = scene.addMaterial(placeholderMat);
	Mesh &planeMesh = scene.addMesh(plane.vertices, plane.indices);

	GameObject &planeobj = scene.addObject("plane", &planeMesh, &plane_mat);
	planeobj.transform.position = glm::vec3(-30.0f, -7.0f, -30.0f);

	std::vector<std::string> faces{
		"textures/skybox01/right.jpg", "textures/skybox01/left.jpg",
		"textures/skybox01/top.jpg",   "textures/skybox01/bottom.jpg",
		"textures/skybox01/front.jpg", "textures/skybox01/back.jpg"
	};

	unsigned int cubemap = h32rf::loadCubemap(faces);

	Skybox &skyobj = scene.getSkybox();
	skyobj.cubemap = cubemap;

	auto &dl = scene.getDirLight();
	dl.direction = glm::vec3(0.0f, -1.0f, -0.3f);
	dl.ambient = glm::vec3(0.01f);
	dl.diffuse = glm::vec3(0.1f);
	dl.specular = glm::vec3(0.0f);

	
	glm::vec3 plightPos[] = {
		glm::vec3(0.0f, 0.0f, -3.0f),	glm::vec3(5.0f, 5.0f, -5.0f),
		glm::vec3(-5.0f, 2.0f, -5.0f),	glm::vec3(0.0f, -2.0f, -10.0f),
		glm::vec3(3.0f, -2.0f, -10.0f), glm::vec3(2.0f, 1.5f, -5.0f),
	};
	glm::vec3 plightDiff[] = {
		glm::vec3(1.0f),	glm::vec3(0.8f),
		glm::vec3(0.9f),	glm::vec3(0.6f),
		glm::vec3(0.8f), glm::vec3(0.5f),
	};


	auto &pl = scene.getPointLights();

	PointLight pl0;
	for (int i = 0; i < 3; i++) {
		pl0.position = plightPos[i];
		pl0.constant = 1.0f;
		pl0.linear = 0.09f;
		pl0.quadratic = 0.032f;
		pl0.ambient = glm::vec3(0.8f);
		pl0.diffuse = plightDiff[i];
		 //glm::vec3(1.0f);
		pl0.specular = glm::vec3(0.3f);
		pl.push_back(pl0);
	}

	placeholderMat.diff = whitetxt.ID;
	placeholderMat.spec = whitetxt.ID;
	Material &plight_mat = scene.addMaterial(placeholderMat);
	
	//GameObject &cubelight = scene.addObject("pl0", &cubeMesh, &plight_mat);
	//cubelight.transform.position = plightPos[0];
	//cubelight.transform.scale = glm::vec3(0.3);

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

