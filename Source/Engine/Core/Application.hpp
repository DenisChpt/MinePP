#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <memory>
#include "Input/InputManager.hpp"
#include "Rendering/Camera.hpp"
#include "GameManager.hpp"

namespace Engine {
namespace Core {

class Application {
public:
	Application();
	~Application();

	int run();

private:
	bool initSDL();
	void mainLoop();
	void cleanup();

	SDL_Window*   m_window;
	SDL_GLContext m_glContext;
	bool          m_running;
	int           m_screenWidth;
	int           m_screenHeight;

	// Input & Camera
	Engine::Input::InputManager m_input;
	Engine::Rendering::Camera   m_camera;

	// Le jeu (GameManager)
	std::unique_ptr<Game::GameManager> m_gameManager;
};

} // namespace Core
} // namespace Engine
