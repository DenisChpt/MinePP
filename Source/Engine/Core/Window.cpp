#include "Window.hpp"
#include <iostream>

Window::Window(const std::string& title, int width, int height) {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}
	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Window::ShouldClose() const {
	return glfwWindowShouldClose(window);
}

void Window::SwapBuffers() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

GLFWwindow* Window::GetGLFWWindow() const {
	return window;
}