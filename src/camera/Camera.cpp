#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch),
	  MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f)
{
	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const {
	return glm::perspective(glm::radians(Zoom), aspectRatio, nearPlane, farPlane);
}

void Camera::processKeyboard(const char direction, float deltaTime) {
	float velocity = MovementSpeed * deltaTime;
	if (direction == 'W') {
		Position += Front * velocity;
	}
	if (direction == 'S') {
		Position -= Front * velocity;
	}
	if (direction == 'A') {
		Position -= Right * velocity;
	}
	if (direction == 'D') {
		Position += Right * velocity;
	}
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw   += xoffset;
	Pitch += yoffset;

	if (constrainPitch) {
		if (Pitch > 89.0f) {
			Pitch = 89.0f;
		}
		if (Pitch < -89.0f) {
			Pitch = -89.0f;
		}
	}
	updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
	Zoom -= yoffset;
	if (Zoom < 1.0f) {
		Zoom = 1.0f;
	}
	if (Zoom > 45.0f) {
		Zoom = 45.0f;
	}
}

void Camera::updateCameraVectors() {
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up    = glm::normalize(glm::cross(Right, Front));
}

void Camera::updateFrustum(const glm::mat4 &viewProjection) {
	FrustumPlanes.clear();
	FrustumPlanes.resize(6);

	// Left plane
	FrustumPlanes[0].normal.x = viewProjection[0][3] + viewProjection[0][0];
	FrustumPlanes[0].normal.y = viewProjection[1][3] + viewProjection[1][0];
	FrustumPlanes[0].normal.z = viewProjection[2][3] + viewProjection[2][0];
	FrustumPlanes[0].distance = viewProjection[3][3] + viewProjection[3][0];

	// Right plane
	FrustumPlanes[1].normal.x = viewProjection[0][3] - viewProjection[0][0];
	FrustumPlanes[1].normal.y = viewProjection[1][3] - viewProjection[1][0];
	FrustumPlanes[1].normal.z = viewProjection[2][3] - viewProjection[2][0];
	FrustumPlanes[1].distance = viewProjection[3][3] - viewProjection[3][0];

	// Bottom plane
	FrustumPlanes[2].normal.x = viewProjection[0][3] + viewProjection[0][1];
	FrustumPlanes[2].normal.y = viewProjection[1][3] + viewProjection[1][1];
	FrustumPlanes[2].normal.z = viewProjection[2][3] + viewProjection[2][1];
	FrustumPlanes[2].distance = viewProjection[3][3] + viewProjection[3][1];

	// Top plane
	FrustumPlanes[3].normal.x = viewProjection[0][3] - viewProjection[0][1];
	FrustumPlanes[3].normal.y = viewProjection[1][3] - viewProjection[1][1];
	FrustumPlanes[3].normal.z = viewProjection[2][3] - viewProjection[2][1];
	FrustumPlanes[3].distance = viewProjection[3][3] - viewProjection[3][1];

	// Near plane
	FrustumPlanes[4].normal.x = viewProjection[0][3] + viewProjection[0][2];
	FrustumPlanes[4].normal.y = viewProjection[1][3] + viewProjection[1][2];
	FrustumPlanes[4].normal.z = viewProjection[2][3] + viewProjection[2][2];
	FrustumPlanes[4].distance = viewProjection[3][3] + viewProjection[3][2];

	// Far plane
	FrustumPlanes[5].normal.x = viewProjection[0][3] - viewProjection[0][2];
	FrustumPlanes[5].normal.y = viewProjection[1][3] - viewProjection[1][2];
	FrustumPlanes[5].normal.z = viewProjection[2][3] - viewProjection[2][2];
	FrustumPlanes[5].distance = viewProjection[3][3] - viewProjection[3][2];

	// Normalize plane equations
	for (int i = 0; i < 6; ++i) {
		float length = glm::length(FrustumPlanes[i].normal);
		if (length != 0.0f) {
			FrustumPlanes[i].normal /= length;
			FrustumPlanes[i].distance /= length;
		}
	}
}

bool Camera::isBoxInFrustum(const glm::vec3 &min, const glm::vec3 &max) const {
	for (int i = 0; i < 6; i++) {
		glm::vec3 p = min;
		if (FrustumPlanes[i].normal.x >= 0) {
			p.x = max.x;
		}
		if (FrustumPlanes[i].normal.y >= 0) {
			p.y = max.y;
		}
		if (FrustumPlanes[i].normal.z >= 0) {
			p.z = max.z;
		}
		if (glm::dot(FrustumPlanes[i].normal, p) + FrustumPlanes[i].distance < 0) {
			return false;
		}
	}
	return true;
}
