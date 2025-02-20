#include "InputManager.hpp"

namespace input {

InputManager::InputManager() : mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0) { }

InputManager::~InputManager() { }

void InputManager::keyCallback(int key, int scancode, int action, int mods) {
	if (action == 1) // press
		keys[key] = true;
	else if (action == 0) // release
		keys[key] = false;
}

void InputManager::mouseCallback(double xpos, double ypos) {
	mouseX = xpos;
	mouseY = ypos;
}

void InputManager::scrollCallback(double xoffset, double yoffset) {
	scrollX = xoffset;
	scrollY = yoffset;
}

void InputManager::update() {
	scrollX = 0.0;
	scrollY = 0.0;
}

bool InputManager::isKeyPressed(int key) const {
	auto it = keys.find(key);
	return (it != keys.end()) ? it->second : false;
}

double InputManager::getMouseX() const {
	return mouseX;
}

double InputManager::getMouseY() const {
	return mouseY;
}

} // namespace input
