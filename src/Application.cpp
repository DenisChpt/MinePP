#include "Application.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Application* Application::instance = nullptr;

Application::Application(int windowWidth, int windowHeight)
	: windowWidth(windowWidth), windowHeight(windowHeight),
	  // Position initiale et cible pour la caméra
	  camera(glm::vec3(0.0f, 20.0f, 50.0f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0,1,0))
{
	// Par exemple, créer un monde de 8x8 chunks
	world = new World(8, 8);
	instance = this;
	memset(keys, 0, sizeof(keys));
}

Application::~Application() {
	delete world;
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Application::init() {
	if (!glfwInit()) {
		std::cerr << "Échec d'initialisation de GLFW" << std::endl;
		return false;
	}
	// Création d'une fenêtre OpenGL 3.3 Core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Mon Projet", nullptr, nullptr);
	if (!window) {
		std::cerr << "Échec de création de la fenêtre" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Initialisation de GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Échec d'initialisation de GLAD" << std::endl;
		return false;
	}

	// Définir les callbacks d'entrée
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	// Masquer le curseur et activer le mode capture (pour la rotation de la caméra)
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialiser le renderer
	if (!renderer.init()) {
		std::cerr << "Échec de l'initialisation du renderer" << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);

	return true;
}

void Application::run() {
	float lastFrame = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		// Gestion simple des entrées (WASD pour avancer/reculer/gauche/droite)
		glm::vec3 direction(0.0f);
		if (keys[GLFW_KEY_W])
			direction += glm::vec3(0, 0, -1);
		if (keys[GLFW_KEY_S])
			direction += glm::vec3(0, 0, 1);
		if (keys[GLFW_KEY_A])
			direction += glm::vec3(-1, 0, 0);
		if (keys[GLFW_KEY_D])
			direction += glm::vec3(1, 0, 0);
		if (glm::length(direction) > 0.0f) {
			direction = glm::normalize(direction);
			camera.move(direction * deltaTime * 10.0f);
		}

		world->update();

		glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.renderWorld(*world, camera);

		glfwSwapBuffers(window);
	}
}

// Callback clavier
void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (instance) {
		if (key >= 0 && key < 1024) {
			if (action == GLFW_PRESS)
				instance->keys[key] = true;
			else if (action == GLFW_RELEASE)
				instance->keys[key] = false;
		}
	}
}

// Callback de position de la souris
void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	static double lastX = xpos, lastY = ypos;
	double deltaX = xpos - lastX;
	double deltaY = lastY - ypos; // inversion de l'axe Y
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	instance->camera.rotate(deltaX * sensitivity, deltaY * sensitivity);
}
