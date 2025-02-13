#include "Application.hpp"
#include <iostream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {
namespace Core {

Application::Application()
	: m_window(nullptr)
	, m_glContext(nullptr)
	, m_running(true)
	, m_screenWidth(1280)
	, m_screenHeight(720)
{
}

Application::~Application() {
	cleanup();
}

bool Application::initSDL()
{
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "[SDL] Init error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
		m_screenWidth = displayMode.w;
		m_screenHeight = displayMode.h;
	} else {
		std::cerr << "[SDL] Erreur lors de la récupération de la résolution de l'écran: "
				  << SDL_GetError() << std::endl;
	}

	// OpenGL 3.3 Core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	m_window = SDL_CreateWindow("Meinkraft",
								SDL_WINDOWPOS_CENTERED,
								SDL_WINDOWPOS_CENTERED,
								m_screenWidth, m_screenHeight,
								SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if(!m_window) {
		std::cerr << "[SDL] Window error: " << SDL_GetError() << std::endl;
		return false;
	}
	m_glContext = SDL_GL_CreateContext(m_window);
	if(!m_glContext) {
		std::cerr << "[SDL] GLContext error: " << SDL_GetError() << std::endl;
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		std::cerr << "[GLEW] Init error: " << glewGetErrorString(err) << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);

	SDL_SetRelativeMouseMode(SDL_TRUE); // mode souris relative

	return true;
}

int Application::run()
{
	if(!initSDL()) {
		return 1;
	}

	// On crée et init le GameManager
	m_gameManager = std::make_unique<Game::GameManager>();
	m_gameManager->init();

	mainLoop();
	return 0;
}

void Application::mainLoop()
{
	auto lastTime = std::chrono::high_resolution_clock::now();

	while(m_running) {
		// ====== EVENTS ======
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(!m_input.processEvent(e)) {
				// SDL_QUIT => false => on quitte
				m_running = false;
			}
			// Gestions de touches globales
			if(e.type == SDL_KEYDOWN) {
				if(e.key.keysym.sym == SDLK_ESCAPE) {
					m_running = false;
				}
				// Changer mode caméra
				else if(e.key.keysym.sym == SDLK_F5) {
					m_camera.setMode(Engine::Rendering::CameraMode::FIRST_PERSON);
				}
				else if(e.key.keysym.sym == SDLK_F6) {
					m_camera.setMode(Engine::Rendering::CameraMode::THIRD_PERSON);
				}
				else if(e.key.keysym.sym == SDLK_F7) {
					m_camera.setMode(Engine::Rendering::CameraMode::THIRD_PERSON_REAR);
				}
			}
		}

		// ====== DELTATIME ======
		auto now = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;


		// On récupère delta souris
		float mouseX = m_input.getMouseDeltaX();
		float mouseY = m_input.getMouseDeltaY();

		// ajuster la caméra (yaw/pitch)
		float sensitivity = 0.1f;
		m_camera.addYawPitch(mouseX * sensitivity, mouseY * sensitivity);

		// ====== UPDATE GAME ======
		m_gameManager->update(dt, m_input, m_camera);

		// ====== UPDATE INPUT ======
		m_input.update(dt);

		// Récupérer la position du joueur pour la caméra
		auto player = m_gameManager->getPlayer();
		glm::vec3 playerPos(0,0,0);
		if(player) {
			playerPos = player->getPosition();
		}

		// Mettre à jour la caméra
		m_camera.updateViewMatrix(playerPos, dt);

		// ====== RENDER ======
		glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = m_camera.getViewMatrix();
		glm::mat4 projection = m_camera.getProjectionMatrix(
			float(m_screenWidth)/float(m_screenHeight)
		);

		// Rendu du Game (on passe view/projection)
		m_gameManager->render(view, projection, m_screenWidth, m_screenHeight);

		// Swap
		SDL_GL_SwapWindow(m_window);
	}

	// On sort de la boucle => shutdown du game
	m_gameManager->shutdown();

}

void Application::cleanup()
{
	if(m_glContext) {
		SDL_GL_DeleteContext(m_glContext);
		m_glContext = nullptr;
	}
	if(m_window) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
	SDL_Quit();
}

} // namespace Core
} // namespace Engine
