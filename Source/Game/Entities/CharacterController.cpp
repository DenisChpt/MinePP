#include "CharacterController.hpp"
#include "Character.hpp"
#include <glm/glm.hpp>
#include <iostream>

namespace Game {

CharacterController::CharacterController(Character& character)
	: m_Character(character),
	  m_MouseSensitivity(0.1f)
{
}

CharacterController::~CharacterController() {
}

void CharacterController::handleKeyboardInput(bool moveForward, bool moveBackward, bool moveLeft, bool moveRight, bool jump, float deltaTime) {
	glm::vec3 direction(0.0f);

	// Pour cet exemple, nous utilisons des directions fixes.
	if (moveForward)
		direction += glm::vec3(0.0f, 0.0f, -1.0f);
	if (moveBackward)
		direction += glm::vec3(0.0f, 0.0f, 1.0f);
	if (moveLeft)
		direction += glm::vec3(-1.0f, 0.0f, 0.0f);
	if (moveRight)
		direction += glm::vec3(1.0f, 0.0f, 0.0f);

	if (jump)
		m_Character.jump();

	if (glm::length(direction) > 0.0f) {
		direction = glm::normalize(direction);
		m_Character.move(direction * deltaTime * 5.0f); // 5.0f : facteur de vitesse
	}
}

void CharacterController::handleMouseMovement(float xoffset, float yoffset) {
	m_Character.addYaw(xoffset * m_MouseSensitivity);
	m_Character.addPitch(yoffset * m_MouseSensitivity);
	std::cout << "[CharacterController] Updated orientation with offsets (" << xoffset << ", " << yoffset << ")." << std::endl;
}

void CharacterController::update(float deltaTime) {
	// Ici, vous pouvez gérer des mises à jour propres au contrôleur (inertie, transitions, etc.)
}

} // namespace Game
