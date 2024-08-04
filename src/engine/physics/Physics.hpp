#include <glm/glm.hpp>
#include "World.hpp"

#ifndef PHYSICS_HPP
#define PHYSICS_HPP

class Physics {
public:
	static const float GRAVITY;

	static void applyGravity(World& world, glm::vec3& position, float& verticalVelocity, float deltaTime, bool& isFlying, bool& isJumping, bool& canGoUp, bool& canGoDown);
};

#endif // PHYSICS_HPP
