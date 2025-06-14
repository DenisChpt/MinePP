// Temporary Camera.hpp for Persistence compatibility
// TODO: Refactor Persistence to work directly with Player

#pragma once

#include "../Common.hpp"

struct MovementDirection
{
	bool isMoving = false;
	glm::vec3 direction = glm::vec3(0);
};

class Camera
{
	glm::mat4 view = glm::mat4(1.0f);
	glm::vec3 position = {14, 100, 17};
	glm::vec3 cameraUp = {0, 1, 0};
	glm::vec3 lookDirection = {1, 0, 0};

	float yaw = 0;
	float pitch = 0.5;

public:
	const glm::mat4 &setPosition(glm::vec3 eye) {
		position = eye;
		return view;
	}
	
	void updateCameraOrientation(float newYaw, float newPitch) {
		yaw = newYaw;
		pitch = newPitch;
	}
	
	[[nodiscard]] float getYaw() const { return yaw; }
	[[nodiscard]] float getPitch() const { return pitch; }
	[[nodiscard]] glm::vec3 getPosition() const { return position; }
};