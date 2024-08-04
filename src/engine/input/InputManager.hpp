#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <GLFW/glfw3.h>
#include <unordered_map>

class InputManager {
public:
	InputManager();
	~InputManager();

	void initialize(GLFWwindow* window);
	void update();

	bool isKeyPressed(int key);
	bool isMouseButtonPressed(int button);
	void getMousePosition(double& x, double& y);
	void getMouseDelta(double& xoffset, double& yoffset);

private:
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

	static std::unordered_map<int, bool> m_keys;
	static std::unordered_map<int, bool> m_mouseButtons;
	static double m_mouseX, m_mouseY;
	static double m_lastMouseX, m_lastMouseY;
	static bool m_firstMouse;
};

#endif // INPUTMANAGER_HPP
