#include "Input.h"

void processInput(GLFWwindow *window, Scene &scene, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	auto &cam = scene.getCamera();

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		std::cout << "ESC Pressed\n";
		glfwSetWindowShouldClose(window, true);
	}
	static bool isF3pressed = false;
	int F3STATE = glfwGetKey(window, GLFW_KEY_F3);
	int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
	if (F3STATE == GLFW_PRESS && !isF3pressed) {
		if (cursorMode == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			// std::cout << "Cursor Unlocked\n";
		} else if (cursorMode == GLFW_CURSOR_NORMAL) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursor(window, nullptr);
			// std::cout << "Cursor Locked\n";
		}
		isF3pressed = true;
	} else if (F3STATE == GLFW_RELEASE) {
		isF3pressed = false;
	}

	static bool isFpressed = false;
	int FSTATE = glfwGetKey(window, GLFW_KEY_F);
	if (FSTATE == GLFW_PRESS && !isFpressed) {
		flashlight_state = (flashlight_state + 1) % 3;
		isFpressed = true;
	} else if (FSTATE == GLFW_RELEASE) {
		isFpressed = false;
	}

	/* if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cam.ProcessKeyboard(CameraMovement::Up, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cam.ProcessKeyboard(CameraMovement::Down, deltaTime);*/
}