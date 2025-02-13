#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Engine {
namespace Rendering {

Camera::Camera()
	: m_Mode(CameraMode::FIRST_PERSON)
	, m_Yaw(90.0f)
	, m_Pitch(0.0f)
	, m_Position(0.0f, 0.0f, 0.0f)
	, m_Front(0.0f, 0.0f, -1.0f)
	, m_Up(0.0f, 1.0f, 0.0f)
	, m_ThirdPersonDistance(8.0f)
{
}

Camera::~Camera() {}

void Camera::setMode(CameraMode mode) {
	m_Mode = mode;
}

CameraMode Camera::getMode() const {
	return m_Mode;
}

void Camera::setYawPitch(float yaw, float pitch) {
	m_Yaw   = yaw;
	m_Pitch = pitch;
}

void Camera::addYawPitch(float dyaw, float dpitch) {
	m_Yaw   += dyaw;
	m_Pitch -= dpitch;
	if(m_Pitch > 89.0f) m_Pitch = 89.0f;
	if(m_Pitch < -89.0f) m_Pitch = -89.0f;
}

void Camera::updateViewMatrix(const glm::vec3& playerPos, float /*dt*/)
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction.y = sin(glm::radians(m_Pitch));
	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction = glm::normalize(direction);

	switch(m_Mode) {
	case CameraMode::FIRST_PERSON: {
		m_Position = playerPos + glm::vec3(0.0f, 1.6f, 0.0f);
		m_Front = direction;
	} break;
	case CameraMode::THIRD_PERSON: {
		glm::vec3 behind = playerPos + glm::vec3(0.0f, 1.0f, 0.0f) - (direction * m_ThirdPersonDistance);
		behind.y += 1.0f;
		m_Position = behind;
		m_Front = glm::normalize(playerPos - m_Position);
	} break;
	case CameraMode::THIRD_PERSON_REAR: {
		glm::vec3 frontPos = playerPos + glm::vec3(0.0f, 1.0f, 0.0f) + (direction * m_ThirdPersonDistance);
		frontPos.y += 1.0f;
		m_Position = frontPos;
		m_Front = glm::normalize(playerPos - m_Position);
	} break;
	}
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const
{
	return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

const glm::vec3& Camera::getPosition() const {
	return m_Position;
}

const glm::vec3& Camera::getFront() const {
	return m_Front;
}

} // namespace Rendering
} // namespace Engine
