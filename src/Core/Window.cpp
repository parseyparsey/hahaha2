#include "Window.h"
#include <iostream>

static void framebufferSizeCallback(GLFWwindow *handle, int width, int height) {
	auto *win = static_cast<Window *>(glfwGetWindowUserPointer(handle));
	win->onResize(width, height);
}

Window::Window(int width, int height, const char *title) :
	m_width(width), m_height(height) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!m_handle) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_handle);
	glfwSetWindowUserPointer(m_handle, this);
	glfwSetFramebufferSizeCallback(m_handle, framebufferSizeCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
	}
}

Window::~Window() { glfwTerminate(); }

bool Window::shouldClose() const { return glfwWindowShouldClose(m_handle); }

void Window::swapBuffers() { glfwSwapBuffers(m_handle); }

void Window::pollEvents() { glfwPollEvents(); }

void Window::onResize(int width, int height) {
	m_width = width;
	m_height = height;
	if (onResizeCallback)
		onResizeCallback(width, height);
}