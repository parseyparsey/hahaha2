#pragma once
#include <GLFW/glfw3.h>
#include <functional>

class Window {
public:
	Window(int width, int height, const char *title);
	~Window();

	bool shouldClose() const;
	void swapBuffers();
	void pollEvents();

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	GLFWwindow *getHandle() const { return m_handle; }

	// called by GLFW's resize callback, updates cached size
	void onResize(int width, int height);

	// optional hook so Engine/Renderer can react to resize without polling
	// every frame
	std::function<void(int, int)> onResizeCallback;

private:
	GLFWwindow *m_handle = nullptr;
	int m_width, m_height;
};