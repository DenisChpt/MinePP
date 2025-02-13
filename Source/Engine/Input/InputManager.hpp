#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>

namespace Engine {
namespace Input {

enum class KeyState {
	UP,
	DOWN,
	PRESSED,
	RELEASED
};

class InputManager {
public:
	InputManager();
	~InputManager();

	void update(float dt);
	bool processEvent(const SDL_Event& event);

	bool isKeyDown(SDL_Scancode key) const;
	bool isKeyPressed(SDL_Scancode key) const;
	bool isKeyReleased(SDL_Scancode key) const;

	float getMouseDeltaX() const;
	float getMouseDeltaY() const;

	// Nouveaux : gestion des boutons souris
	bool isMouseButtonDown(Uint8 button) const;
	bool isMouseButtonPressed(Uint8 button) const;
	bool isMouseButtonReleased(Uint8 button) const;
	KeyState getMouseButtonState(Uint8 button) const;

private:
	std::unordered_map<SDL_Scancode, KeyState> m_keyStates;
	std::unordered_map<Uint8, KeyState> m_mouseButtonStates;
	float m_mouseDeltaX;
	float m_mouseDeltaY;
};

} // namespace Input
} // namespace Engine
