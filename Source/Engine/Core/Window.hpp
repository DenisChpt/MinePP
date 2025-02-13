#pragma once
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
	Window(const std::string& title, int width, int height);
	~Window();
	bool ShouldClose() const;
	void SwapBuffers();
	GLFWwindow* GetGLFWWindow() const;

private:
	GLFWwindow* window;
};