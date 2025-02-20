#include <glad/glad.h>
#include "Application.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/PostProcessor.hpp"
#include "camera/Camera.hpp"
#include "ui/ChatWindow.hpp"
#include "ui/DebugOverlay.hpp"
#include "input/InputManager.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

namespace {
	Application* appInstance = nullptr;
}

Application::Application(int width, int height, const std::string &title)
	: screenWidth(width), screenHeight(height), windowTitle(title), window(nullptr),
	  renderer(new renderer::Renderer()),
	  postProcessor(new renderer::PostProcessor()),
	  camera(new Camera(glm::vec3(0.0f, 50.0f, 100.0f))),
	  chatWindow(new ui::ChatWindow()),
	  debugOverlay(new ui::DebugOverlay()),
	  inputManager(new input::InputManager())
{
	appInstance = this;
}

Application::~Application() {
	delete renderer;
	delete postProcessor;
	delete camera;
	delete chatWindow;
	delete debugOverlay;
	delete inputManager;
	glfwTerminate();
}

bool Application::initialize() {
	if (!glfwInit()) {
		std::cerr << "Erreur: Échec de l'initialisation de GLFW." << std::endl;
		return false;
	}
	window = glfwCreateWindow(screenWidth, screenHeight, windowTitle.c_str(), nullptr, nullptr);
	if (!window) {
		std::cerr << "Erreur: Échec de la création de la fenêtre GLFW." << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallbackStatic);
	glfwSetCursorPosCallback(window, mouseCallbackStatic);
	glfwSetScrollCallback(window, scrollCallbackStatic);
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		std::cerr << "Erreur: Échec de l'initialisation de GLAD." << std::endl;
		return false;
	}
	glViewport(0, 0, screenWidth, screenHeight);
	glEnable(GL_DEPTH_TEST);

	if (!renderer->initialize()) {
		std::cerr << "Erreur: Échec de l'initialisation du renderer." << std::endl;
		return false;
	}
	if (!postProcessor->initialize(screenWidth, screenHeight)) {
		std::cerr << "Erreur: Échec de l'initialisation du post processor." << std::endl;
		return false;
	}
	return true;
}

void Application::run() {
	auto lastTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window)) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		processInput(deltaTime);
		update(deltaTime);

		// Rendu de la scène dans le framebuffer du post-processor
		postProcessor->bindFramebuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 view = camera->getViewMatrix();
		glm::mat4 projection = camera->getProjectionMatrix((float)screenWidth / screenHeight);
		renderer->render(view, projection);

		// Passage de post‑traitement
		postProcessor->render((float)glfwGetTime());

		// Rendu des overlays UI
		debugOverlay->setFPS(1.0f / deltaTime);
		debugOverlay->setCameraPosition(camera->Position);
		debugOverlay->render(projection);
		if (chatWindow->isActive())
			chatWindow->render(projection);

		glfwSwapBuffers(window);
		glfwPollEvents();
		inputManager->update();
	}
}

void Application::processInput(float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (!chatWindow->isActive()) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera->processKeyboard('W', deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera->processKeyboard('S', deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera->processKeyboard('A', deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera->processKeyboard('D', deltaTime);
	}
}

void Application::update(float deltaTime) {
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix((float)screenWidth / screenHeight);
	camera->updateFrustum(projection * view);
}

GLFWwindow* Application::getWindow() const {
	return window;
}

void Application::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (appInstance) {
		appInstance->inputManager->keyCallback(key, scancode, action, mods);
		if (appInstance->chatWindow->isActive())
			appInstance->chatWindow->processInput(key, action);
		if (key == GLFW_KEY_T && action == GLFW_PRESS)
			appInstance->chatWindow->toggle();
	}
}

void Application::mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
	if (appInstance) {
		appInstance->inputManager->mouseCallback(xpos, ypos);
		if (!appInstance->chatWindow->isActive()) {
			static double lastX = xpos, lastY = ypos;
			double xoffset = xpos - lastX;
			double yoffset = lastY - ypos;
			lastX = xpos;
			lastY = ypos;
			appInstance->camera->processMouseMovement((float)xoffset, (float)yoffset);
		}
	}
}

void Application::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
	if (appInstance) {
		appInstance->inputManager->scrollCallback(xoffset, yoffset);
		appInstance->camera->processMouseScroll((float)yoffset);
	}
}
