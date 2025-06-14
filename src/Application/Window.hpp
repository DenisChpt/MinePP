/**
 * @class Window
 * @brief Encapsule la gestion de la fenêtre et du contexte OpenGL.
 *
 * @details La classe Window initialise GLFW, configure le contexte OpenGL (version 4.6 Core
 * profile), gère les callbacks (clavier, souris, redimensionnement, erreurs), et propose des
 * méthodes pour verrouiller/déverrouiller la souris, démarrer/terminer une frame et échanger les
 * buffers.
 *
 * @param windowWidth Largeur initiale de la fenêtre.
 * @param windowHeight Hauteur initiale de la fenêtre.
 *
 * @note Des fonctionnalités avancées telles que l'affichage de messages de débogage OpenGL sont
 * également implémentées.
 */

#pragma once

#include "../Common.hpp"
#include "../Rendering/Framebuffers.hpp"

class Application;

class Window {
	const char* name = "MinePP";
	int32_t windowWidth = 1200;
	int32_t windowHeight = 900;
	GLFWwindow* window = nullptr;
	glm::vec4 clearColor = {0, 0, 0, 1};
	Ref<FramebufferStack> framebufferStack = std::make_shared<FramebufferStack>();
	Application* applicationPtr = nullptr;
	class Assets* assetsPtr = nullptr;

	// GUI management (from Gui class)
	void initGui();
	void shutdownGui();

	void setupCallbacks();
	static bool setupGlad();

	static void onKeyEvent(
		GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mode);
	static void onResized(GLFWwindow* window, int32_t width, int32_t height);
	static void onMouseButtonEvent(GLFWwindow* window,
								   int32_t button,
								   int32_t action,
								   int32_t mods);
	static void onCursorPosition(GLFWwindow* window, double x, double y);
	static void onRefreshWindow(GLFWwindow* window);

	static void onWindowError(int32_t errorCode, const char* description);
	static void onOpenGlMessage(GLenum source,
								GLenum type,
								GLuint id,
								GLenum severity,
								GLsizei length,
								const GLchar* message,
								const void* userParam);

   public:
	Window();
	~Window();

	void setApplication(Application* app) { applicationPtr = app; }
	void setAssets(Assets* am) { assetsPtr = am; }

	[[nodiscard]] inline int32_t getWindowWidth() const { return windowWidth; }
	void setWindowWidth(int32_t width) { windowWidth = width; }

	[[nodiscard]] inline int32_t getWindowHeight() const { return windowHeight; }
	void setWindowHeight(int32_t height) { windowHeight = height; }

	[[nodiscard]] inline GLFWwindow* getContext() { return window; };
	[[nodiscard]] inline Ref<FramebufferStack> getFramebufferStack() { return framebufferStack; };

	bool isValid() { return window != nullptr; };
	[[nodiscard]] inline bool shouldClose() const { return glfwWindowShouldClose(window); };

	void beginFrame();
	void resetFrame();
	void finalizeFrame();
	void swapBuffers();

	// GUI frame management (from Gui class)
	void beginGuiFrame();
	void finalizeGuiFrame();

	void pollEvents();
	void unlockMouse();
	void lockMouse();
	glm::dvec2 getCursorPosition();

	Window(const Window&) = delete;
	Window(Window&) = delete;
	Window(Window&&) noexcept = delete;
	Window& operator=(Window&) = delete;
	Window& operator=(Window&&) noexcept = delete;
	bool shouldRender();
};
