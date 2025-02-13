#include "InputManager.hpp"
#include <iostream>

namespace Engine {
namespace Input {

InputManager::InputManager()
	: m_mouseDeltaX(0.0f)
	, m_mouseDeltaY(0.0f)
{
}

InputManager::~InputManager() { }

void InputManager::update(float /*dt*/)
{
	// Met à jour l'état des touches
	for (auto& kv : m_keyStates) {
		if (kv.second == KeyState::PRESSED)
			kv.second = KeyState::DOWN;
		else if (kv.second == KeyState::RELEASED)
			kv.second = KeyState::UP;
	}
	for (auto& kv : m_mouseButtonStates) {
		if (kv.second == KeyState::PRESSED)
			kv.second = KeyState::DOWN;
		else if (kv.second == KeyState::RELEASED)
			kv.second = KeyState::UP;
	}
	// Réinitialiser les deltas souris pour le prochain frame
	m_mouseDeltaX = 0.0f;
	m_mouseDeltaY = 0.0f;
}

bool InputManager::processEvent(const SDL_Event& event)
{
	switch(event.type) {
		case SDL_QUIT:
			return false;

		case SDL_KEYDOWN: {
			SDL_Scancode sc = event.key.keysym.scancode;
			if (m_keyStates.find(sc) == m_keyStates.end() || m_keyStates[sc] == KeyState::UP)
				m_keyStates[sc] = KeyState::PRESSED;
		} break;

		case SDL_KEYUP: {
			SDL_Scancode sc = event.key.keysym.scancode;
			m_keyStates[sc] = KeyState::RELEASED;
		} break;

		case SDL_MOUSEMOTION: {
			m_mouseDeltaX += float(event.motion.xrel);
			m_mouseDeltaY += float(event.motion.yrel);
		} break;

		case SDL_MOUSEBUTTONDOWN: {
			Uint8 button = event.button.button;
			// Vérifier si le bouton était en UP ou inconnu, alors on met à PRESSED
			if (m_mouseButtonStates.find(button) == m_mouseButtonStates.end() || m_mouseButtonStates[button] == KeyState::UP)
				m_mouseButtonStates[button] = KeyState::PRESSED;
		} break;



		case SDL_MOUSEBUTTONUP: {
			Uint8 button = event.button.button;
			m_mouseButtonStates[button] = KeyState::RELEASED;
		} break;

		default:
			break;
	}
	return true;
}

bool InputManager::isKeyDown(SDL_Scancode key) const {
	auto it = m_keyStates.find(key);
	return (it != m_keyStates.end() && (it->second == KeyState::DOWN || it->second == KeyState::PRESSED));
}

bool InputManager::isKeyPressed(SDL_Scancode key) const {
	auto it = m_keyStates.find(key);
	return (it != m_keyStates.end() && it->second == KeyState::PRESSED);
}

bool InputManager::isKeyReleased(SDL_Scancode key) const {
	auto it = m_keyStates.find(key);
	return (it != m_keyStates.end() && it->second == KeyState::RELEASED);
}

float InputManager::getMouseDeltaX() const { return m_mouseDeltaX; }
float InputManager::getMouseDeltaY() const { return m_mouseDeltaY; }

bool InputManager::isMouseButtonDown(Uint8 button) const {
	auto it = m_mouseButtonStates.find(button);
	return (it != m_mouseButtonStates.end() && (it->second == KeyState::DOWN || it->second == KeyState::PRESSED));
}

bool InputManager::isMouseButtonPressed(Uint8 button) const {
	auto it = m_mouseButtonStates.find(button);
	return (it != m_mouseButtonStates.end() && it->second == KeyState::PRESSED);
}

bool InputManager::isMouseButtonReleased(Uint8 button) const {
	auto it = m_mouseButtonStates.find(button);
	return (it != m_mouseButtonStates.end() && it->second == KeyState::RELEASED);
}

} // namespace Input
} // namespace Engine
