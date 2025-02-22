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

/**
 * @brief Manages the main application lifecycle: window creation, game loop, and rendering.
 */
class Application {
public:
	/**
	 * @param width The initial window width.
	 * @param height The initial window height.
	 * @param title The window title.
	 */
	Application(int width, int height, const std::string &title);
	~Application();

	/**
	 * @brief Initializes the GLFW context, GLAD, and other subsystems.
	 * @return True if initialization succeeded, otherwise false.
	 */
	bool initialize();

	/**
	 * @brief Starts the main application loop, running until window close.
	 */
	void run();

	/**
	 * @brief Provides access to the underlying GLFWwindow.
	 */
	GLFWwindow* getWindow() const;

	/**
	 * @brief Static callback for keyboard events from GLFW.
	 */
	static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);

	/**
	 * @brief Static callback for mouse movement events from GLFW.
	 */
	static void mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos);

	/**
	 * @brief Static callback for scroll events from GLFW.
	 */
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
