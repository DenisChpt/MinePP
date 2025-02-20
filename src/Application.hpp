#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Renderer.hpp"
#include "renderer/PostProcessor.hpp"
#include "camera/Camera.hpp"
#include "ui/ChatWindow.hpp"
#include "ui/DebugOverlay.hpp"
#include "input/InputManager.hpp"

class Application {
public:
	Application(int width, int height, const std::string &title);
	~Application();

	bool initialize();
	void run();
	GLFWwindow* getWindow() const;

	// Callbacks statiques pour GLFW
	static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);

private:
	int screenWidth;
	int screenHeight;
	std::string windowTitle;
	GLFWwindow* window;

	renderer::Renderer* renderer;
	renderer::PostProcessor* postProcessor;
	Camera* camera;
	ui::ChatWindow* chatWindow;
	ui::DebugOverlay* debugOverlay;
	input::InputManager* inputManager;

	void processInput(float deltaTime);
	void update(float deltaTime);
};

#endif // APPLICATION_HPP
