#include "Camera.hpp"
#include "../physics/Physics.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: front(glm::vec3(0.0f, 0.0f, -1.0f)), mouseSensitivity(0.06f), isFlying(true), isJumping(false),verticalVelocity(0.0f), canGoUp(true), canGoDown(true)
{
	this->movementSpeed = isFlying ? flySpeed : walkSpeed;
	this->position = position;
	this->worldUp = up;
	this->yaw = yaw;
	this->pitch = pitch;
	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(int direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));

	if (direction == 0) // forward
		position += horizontalFront * velocity;
	if (direction == 1) // backward
		position -= horizontalFront * velocity;
	if (direction == 2) // left
		position -= right * velocity;
	if (direction == 3) // right
		position += right * velocity;
	if (isFlying)
	{
		if (direction == 4 && canGoUp) // up
			position += worldUp * velocity;
		if (direction == 5 && canGoDown) // down
			position -= worldUp * velocity;
	}
}

void Camera::processMouseMovement(float xoffset, float yoffset)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	updateCameraVectors();
}

void Camera::update(World &world, float deltaTime)
{
	Physics::applyGravity(world, position, verticalVelocity, deltaTime, isFlying, isJumping, canGoUp, canGoDown);
	movementSpeed = isFlying ? flySpeed : walkSpeed;
}

void Camera::jump()
{
	if (!isFlying && verticalVelocity == 0.0f)
	{
		verticalVelocity = 9.0f;
		isJumping = true;
	}
}
void Camera::setFlying(bool flying)
{
	isFlying = flying;
}

void Camera::updateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}