#include "InputManager.hpp"

std::unordered_map<int, bool> InputManager::m_keys;
std::unordered_map<int, bool> InputManager::m_mouseButtons;
double InputManager::m_mouseX = 0.0;
double InputManager::m_mouseY = 0.0;
double InputManager::m_lastMouseX = 0.0;
double InputManager::m_lastMouseY = 0.0;
bool InputManager::m_firstMouse = true;

InputManager::InputManager() {}

InputManager::~InputManager() {}

void InputManager::initialize(GLFWwindow* window) {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
}

void InputManager::update() {
	// This function can be expanded to include more complex input handling logic
}

bool InputManager::isKeyPressed(int key) {
	return m_keys[key];
}

bool InputManager::isMouseButtonPressed(int button) {
	return m_mouseButtons[button];
}

void InputManager::getMousePosition(double& x, double& y) {
	x = m_mouseX;
	y = m_mouseY;
}

void InputManager::getMouseDelta(double& xoffset, double& yoffset) {
	static double lastX = m_mouseX;
	static double lastY = m_mouseY;

	xoffset = m_mouseX - lastX;
	yoffset = lastY - m_mouseY;

	lastX = m_mouseX;
	lastY = m_mouseY;
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		m_keys[key] = true;
	} else if (action == GLFW_RELEASE) {
		m_keys[key] = false;
	}
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		m_mouseButtons[button] = true;
	} else if (action == GLFW_RELEASE) {
		m_mouseButtons[button] = false;
	}
}

void InputManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	if (m_firstMouse) {
		m_lastMouseX = xpos;
		m_lastMouseY = ypos;
		m_firstMouse = false;
	}

	m_mouseX = xpos;
	m_mouseY = ypos;
}