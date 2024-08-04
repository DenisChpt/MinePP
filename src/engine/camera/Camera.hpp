#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../world/World.hpp"

class Camera
{
public:
	Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

	glm::mat4 getViewMatrix() const;

	void processKeyboard(int direction, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset);
	void update(World &world, float deltaTime);
	void jump();
	void setFlying(bool flying);
	bool getFlying() { return isFlying; };

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float movementSpeed;
	const float walkSpeed = 3.5f;
	const float flySpeed = 10.92f;
	float mouseSensitivity;
	float verticalVelocity;

	bool isFlying;
	bool isJumping;
	bool canGoUp;
	bool canGoDown;

	void updateCameraVectors();
};

#endif // CAMERA_HPP
