#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include "InputManager.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "World.hpp"
#include "Camera.hpp"

const GLchar *vertexSource = R"glsl(
	#version 330 core

	layout (location = 0) in vec3 position;
	layout (location = 1) in vec2 texCoords;

	out vec2 TexCoords;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		TexCoords = texCoords;
		gl_Position = projection * view * model * vec4(position, 1.0);
	}
)glsl";

const GLchar *fragmentSource = R"glsl(
	#version 330 core
	in vec2 TexCoords;
	out vec4 color;

	uniform sampler2D texture1;

	void main() {
		color = texture(texture1, TexCoords);
	}
)glsl";

glm::mat4 projection;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.0f);
}

void processInput(GLFWwindow *window, Camera &camera, float deltaTime, double &lastSpacePressTime, bool &spacePressedOnce)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(0, deltaTime); // forward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(1, deltaTime); // backward
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(2, deltaTime); // left
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(3, deltaTime); // right

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (!spacePressedOnce)
		{
			double currentTime = glfwGetTime();
			if (currentTime - lastSpacePressTime < 0.3)
			{
				camera.setFlying(!camera.getFlying());
				spacePressedOnce = true;
			}
			else
			{
				camera.jump();
				lastSpacePressTime = currentTime;
				spacePressedOnce = true;
			}
		}
		if (camera.getFlying())
		{
			camera.processKeyboard(4, deltaTime);
		}
	}
	else
	{
		spacePressedOnce = false; // Reset if space is released
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if (camera.getFlying())
		{
			camera.processKeyboard(5, deltaTime); // down in flying mode
		}
	}
}

int main()
{
	// Initialisation de GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// Obtenez la taille de l'écran
	GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);

	// Création de la fenêtre GLFW avec la taille de l'écran
	GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Meinkraft", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Capture et cache le curseur de la souris
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Création du contexte OpenGL
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	if (!GLEW_VERSION_3_3)
	{
		std::cerr << "OpenGL 3.3 not supported" << std::endl;
		return -1;
	}

	// Activation du test de profondeur
	glEnable(GL_DEPTH_TEST);

	// Initialisation de l'InputManager
	InputManager inputManager;
	inputManager.initialize(window);

	// Initialisation du Renderer
	Renderer renderer;
	renderer.initialize();

	// Compilation des shaders
	Shader shader;
	shader.compile(vertexSource, fragmentSource);

	// Lier l'unité de texture
	shader.use();
	shader.setInt("texture1", 0); // Assigner l'unité de texture 0

	// Création du monde
	World world;

	// Initialisation de la caméra
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

	// Initialisation de la matrice de projection
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.0f);

	float lastFrame = 0.0f;
	float deltaTime = 0.0f;

	// Variables pour calculer les FPS
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// Variables pour le double appui sur Espace
	double lastSpacePressTime = 0.0;
	bool spacePressedOnce = false;

	// Boucle principale du rendu
	while (!glfwWindowShouldClose(window))
	{
		// Calcul du temps écoulé entre les images
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Calcul des FPS
		nbFrames++;
		if (currentFrame - lastTime >= 1.0)
		{
			std::stringstream ss;
			ss << "Meinkraft - " << nbFrames << " FPS";
			glfwSetWindowTitle(window, ss.str().c_str());
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Gestion des entrées
		glfwPollEvents();
		inputManager.update();
		processInput(window, camera, deltaTime, lastSpacePressTime, spacePressedOnce);

		// Récupère les décalages de la souris
		double xoffset, yoffset;
		inputManager.getMouseDelta(xoffset, yoffset);
		camera.processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));

		// Mise à jour de la caméra (gravité et saut)
		camera.update(world, deltaTime);

		// Rendu
		renderer.clear();

		glm::mat4 view = camera.getViewMatrix();

		shader.use();
		shader.setMatrix4("view", view);
		shader.setMatrix4("projection", projection);

		// Rendu du monde
		world.render(shader);
		// Échange des buffers
		glfwSwapBuffers(window);
	}

	// Nettoyage
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
