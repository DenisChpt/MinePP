#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

/**
 * @brief A simple camera class that handles position, orientation, and frustum.
 */
class Camera {
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	struct Plane {
		glm::vec3 normal;
		float distance;
	};
	std::vector<Plane> FrustumPlanes;

	/**
	 * @brief Creates a new Camera with given initial parameters.
	 */
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
		   glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		   float yaw = -90.0f,
		   float pitch = 0.0f);

	/**
	 * @brief Computes the view matrix based on camera parameters.
	 */
	glm::mat4 getViewMatrix() const;

	/**
	 * @brief Computes a perspective projection matrix with the specified aspect ratio.
	 */
	glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

	/**
	 * @brief Processes keyboard input for movement.
	 */
	void processKeyboard(const char direction, float deltaTime);

	/**
	 * @brief Processes mouse movement input.
	 */
	void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	/**
	 * @brief Processes mouse wheel scroll to adjust zoom.
	 */
	void processMouseScroll(float yoffset);

	/**
	 * @brief Updates the view frustum planes based on a view-projection matrix.
	 */
	void updateFrustum(const glm::mat4 &viewProjection);

	/**
	 * @brief Tests whether an axis-aligned bounding box is within the camera frustum.
	 */
	bool isBoxInFrustum(const glm::vec3 &min, const glm::vec3 &max) const;

private:
	/**
	 * @brief Recalculates camera direction vectors.
	 */
	void updateCameraVectors();
};

#endif // CAMERA_HPP
