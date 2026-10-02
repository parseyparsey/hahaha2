#pragma once
#include "Scene/Scene.h"
#include <GLFW/glfw3.h>

inline int flashlight_state = 0;

void processInput(GLFWwindow *window, Scene &scene, float deltaTime);