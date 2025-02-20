#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Camera {
public:
	// Attributs de la caméra
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Angles Euler
	float Yaw;
	float Pitch;

	// Options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Structure pour une face du frustum
	struct Plane {
		glm::vec3 normal;
		float distance;
	};
	std::vector<Plane> FrustumPlanes;

	// Constructeur
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), 
		   glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
		   float yaw = -90.0f, float pitch = 0.0f);

	// Renvoie la matrice de vue
	glm::mat4 getViewMatrix() const;

	// Renvoie la matrice de projection
	glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

	// Traitement des entrées clavier
	void processKeyboard(const char direction, float deltaTime);

	// Traitement des mouvements de la souris
	void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	// Traitement du scroll
	void processMouseScroll(float yoffset);

	// Met à jour les plans du frustum à partir d'une matrice vue‑projection
	void updateFrustum(const glm::mat4 &viewProjection);

	// Vérifie si une AABB est dans le frustum
	bool isBoxInFrustum(const glm::vec3 &min, const glm::vec3 &max) const;

private:
	// Calcule le vecteur Front à partir des angles Euler
	void updateCameraVectors();
};

#endif // CAMERA_HPP
