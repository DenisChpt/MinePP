// src/engine/physics/Physics.cpp

#include "Physics.hpp"
#include <glm/glm.hpp>

const float Physics::GRAVITY = -23.31f;

void Physics::applyGravity(World& world, glm::vec3& position, float& verticalVelocity, float deltaTime, bool& isFlying, bool& isJumping, bool& canGoUp, bool& canGoDown) {
	canGoDown = true;
	canGoUp = true;
	if (world.checkFall(position - glm::vec3(0.0f, 3.0f, 0.0f)) && !isJumping) {
		canGoDown = false;
		verticalVelocity = 0.0f;
		return;
	}
	if (!isFlying) {
		verticalVelocity += GRAVITY * deltaTime;
		position.y += verticalVelocity * deltaTime;
		isJumping = false;
	}
}