#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class UIManager {
public:
	UIManager(GLFWwindow *ctx);
	~UIManager();

	UIManager(const UIManager &) = delete;
	UIManager &operator=(const UIManager &) = delete;
	
	template <typename T> getVar(T& var);

	void initIMGUI(GLFWwindow *ctx);
	void updateGUI();

private:
};