#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Engine/ResourceManager/AtlasGenerator.hpp"

namespace Game {

class Character {
public:
	Character(const glm::vec3& position);
	~Character();

	// Déplacement
	void move(const glm::vec3& delta);
	void jump();
	void toggleFlying();

	// Mise à jour (physique, animations, etc.)
	void update(float deltaTime);

	// Calcul de la matrice de vue pour la caméra
	glm::mat4 getViewMatrix() const;

	// Modification de l'orientation
	void addYaw(float offset);
	void addPitch(float offset);

	const glm::vec3& getPosition() const;

	/**
	 * @brief Rend le personnage complet en 3D en utilisant l'atlas.
	 * La texture "character" (64×64) est supposée avoir été intégrée dans l'atlas.
	 * Les coordonnées pour chaque partie (tête, corps, bras, jambes) sont définies selon le CSV fourni.
	 * @param atlas L'atlas contenant la texture "character".
	 */
	void render(const Atlas& atlas);

	// Version de debug (affichage console)
	void render();

private:
	void updateDirection();

	glm::vec3 m_Position;
	glm::vec3 m_Front;
	glm::vec3 m_Up;
	float m_Yaw;
	float m_Pitch;
	float m_Speed;
	bool m_IsFlying;
};

} // namespace Game
