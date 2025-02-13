#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {
namespace Rendering {

// Modes de caméra
enum class CameraMode {
	FIRST_PERSON,
	THIRD_PERSON,
	THIRD_PERSON_REAR
};

class Camera {
public:
	Camera();
	~Camera();

	void setMode(CameraMode mode);
	CameraMode getMode() const;

	void setYawPitch(float yaw, float pitch);
	void addYawPitch(float yawOff, float pitchOff);

	void updateViewMatrix(const glm::vec3& playerPos, float dt);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix(float aspect) const;

	const glm::vec3& getPosition() const;
	const glm::vec3& getFront() const;

private:
	CameraMode m_Mode;
	float m_Yaw;
	float m_Pitch;
	glm::vec3 m_Position;
	glm::vec3 m_Front;
	glm::vec3 m_Up;

	float m_ThirdPersonDistance;
};

} // namespace Rendering
} // namespace Engine
