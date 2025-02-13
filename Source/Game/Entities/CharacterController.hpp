#pragma once

namespace Game {

class Character;

class CharacterController {
public:
	CharacterController(Character& character);
	~CharacterController();

	// Traite les entrées clavier (booléens simulant les touches)
	void handleKeyboardInput(bool moveForward, bool moveBackward, bool moveLeft, bool moveRight, bool jump, float deltaTime);
	// Traite les mouvements de la souris
	void handleMouseMovement(float xoffset, float yoffset);
	void update(float deltaTime);

private:
	Character& m_Character;
	float m_MouseSensitivity;
};

} // namespace Game
