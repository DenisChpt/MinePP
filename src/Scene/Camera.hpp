/**
 * @class Camera
 * @brief Gère la vue et l'orientation de la caméra dans le monde.
 *
 * @details La classe Camera fournit des méthodes pour positionner la caméra, calculer la matrice de vue (via glm::lookAt),
 *          et gérer l'orientation (yaw, pitch). Elle propose aussi des getters pour la position, la direction de regard
 *          et le calcul d'un vecteur de déplacement en fonction des commandes utilisateur.
 */


#pragma once

#include "../Common.hpp"

struct MovementDirection
{
	bool isMoving = false;
	glm::vec3 direction = glm::vec3(0);
};

class Camera
{
	glm::mat4 view = calcView();
	glm::vec3 position = {14, 100, 17};
	glm::vec3 cameraUp = {0, 1, 0};

	MovementDirection forward = {false, {1, 0, 0}};
	MovementDirection backward = {false, {-1, 0, 0}};
	MovementDirection left = {false, {0, 0, 1}};
	MovementDirection right = {false, {0, 0, -1}};
	MovementDirection up = {false, {0, 1, 0}};
	MovementDirection down = {false, {0, -1, 0}};
	glm::vec3 lookDirection = forward.direction;

	float yaw = 0;
	float pitch = 0.5;

	glm::mat4 calcView() const;
	const glm::mat4 &updateView();

public:
	const glm::mat4 &lookAt(glm::vec3 eye, glm::vec3 center);

	void updateCameraDirection(glm::vec3 newForward);
	void updateCameraOrientation(float yaw, float pitch);

	[[nodiscard]] const glm::mat4 &getViewMatrix() const { return view; }

	[[nodiscard]] float getYaw() const { return yaw; };
	[[nodiscard]] float getPitch() const { return pitch; };

	[[nodiscard]] glm::vec3 getLookDirection() const;
	[[nodiscard]] glm::vec3 getPosition() const;
	const glm::mat4 &setPosition(glm::vec3 eye);

	void setIsMovingForward(bool isMoving) { forward.isMoving = isMoving; };
	void setIsMovingBackward(bool isMoving) { backward.isMoving = isMoving; };
	void setIsMovingLeft(bool isMoving) { left.isMoving = isMoving; };
	void setIsMovingRight(bool isMoving) { right.isMoving = isMoving; };
	void setIsMovingUp(bool isMoving) { up.isMoving = isMoving; };
	void setIsMovingDown(bool isMoving) { down.isMoving = isMoving; };
	glm::vec3 getMoveDirection();
};
