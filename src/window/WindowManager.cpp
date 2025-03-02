#include "WindowManager.hpp"
#include <iostream>
#include <cstdlib>

// Variables statiques pour stocker les callbacks utilisateur
namespace WindowManager {

static KeyCallbackFunc userKeyCallback = nullptr;
static CharCallbackFunc userCharCallback = nullptr;
static MouseButtonCallbackFunc userMouseButtonCallback = nullptr;
static ScrollCallbackFunc userScrollCallback = nullptr;

// Fonctions internes appelées par GLFW, qui délèguent aux callbacks utilisateur
static void internalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (userKeyCallback)
		userKeyCallback(window, key, scancode, action, mods);
}

static void internalCharCallback(GLFWwindow* window, unsigned int codepoint) {
	if (userCharCallback)
		userCharCallback(window, codepoint);
}

static void internalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (userMouseButtonCallback)
		userMouseButtonCallback(window, button, action, mods);
}

static void internalScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (userScrollCallback)
		userScrollCallback(window, xoffset, yoffset);
}

void setKeyCallback(KeyCallbackFunc callback) {
	userKeyCallback = callback;
}

void setCharCallback(CharCallbackFunc callback) {
	userCharCallback = callback;
}

void setMouseButtonCallback(MouseButtonCallbackFunc callback) {
	userMouseButtonCallback = callback;
}

void setScrollCallback(ScrollCallbackFunc callback) {
	userScrollCallback = callback;
}

GLFWwindow* createWindow(const Config &config) {
	int windowWidth = config.getWindowWidth();
	int windowHeight = config.getWindowHeight();
	GLFWmonitor *monitor = nullptr;
	if (config.isFullscreen()) {
		monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		if (mode) {
			windowWidth = mode->width;
			windowHeight = mode->height;
		}
	}
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "MinePP", monitor, nullptr);
	if (!window) {
		std::cerr << "Erreur : Impossible de créer la fenêtre GLFW." << std::endl;
		return nullptr;
	}
	return window;
}

void setupCallbacks(GLFWwindow *window) {
	glfwSetKeyCallback(window, internalKeyCallback);
	glfwSetCharCallback(window, internalCharCallback);
	glfwSetMouseButtonCallback(window, internalMouseButtonCallback);
	glfwSetScrollCallback(window, internalScrollCallback);
}

} // namespace WindowManager
