#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "camera/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "world/World.hpp"
#include <GLFW/glfw3.h>

class Application {
public:
	Application(int windowWidth, int windowHeight);
	~Application();

	bool init();
	void run();

private:
	int windowWidth, windowHeight;
	GLFWwindow* window;
	Renderer renderer;
	Camera camera;
	World* world;

	// Tableau d'états clavier
	bool keys[1024];

	// Callbacks GLFW pour la gestion des entrées
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

	// Pointeur statique pour accéder à l'instance dans les callbacks
	static Application* instance;
};

#endif // APPLICATION_HPP
