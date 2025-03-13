/**
 * @class AxisPlane
 * @brief Calcule l'intersection d'un rayon avec un plan aligné sur l'un des axes.
 *
 * @details La classe AxisPlane, initialisée avec la normale du plan, la position et la direction du rayon,
 *          calcule la position d'intersection, la distance et fournit une méthode pour avancer l'offset du plan.
 *
 * @param planeNormal Vecteur normal du plan.
 * @param rayPosition Position de départ du rayon.
 * @param rayDirection Direction du rayon.
 *
 * @return La distance (t) d'intersection ou -infinity si parallèle.
 */


#pragma once

#include "../MinePP.hpp"

class AxisPlane
{
	glm::vec3 planeNormal;

	float offsetDirection;
	float planeOffset;

	glm::vec3 rayPosition;
	glm::vec3 rayDirection;

	glm::vec3 hitPosition;
	float hitDistance;

	[[nodiscard]] float intersect() const;

	[[nodiscard]] float calculateOffsetDirection(const glm::vec3 &direction) const;
	[[nodiscard]] float calculateStartOffset(const glm::vec3 &position, const glm::vec3 &direction) const;

	[[nodiscard]] float calculateHitDistanceToPosition() const { return glm::distance(rayPosition, hitPosition); }
	[[nodiscard]] glm::vec3 calculateHitPosition() const;

public:
	static std::optional<glm::ivec3> rayHitsToBlockPosition(const glm::vec3 &hit1, const glm::vec3 &hit2);
	AxisPlane(glm::vec3 planeNormal, glm::vec3 rayPosition, glm::vec3 rayDirection);

	[[nodiscard]] glm::vec3 getHitPosition() const { return hitPosition; };
	[[nodiscard]] float getHitDistance() const { return hitDistance; };

	bool operator<(const AxisPlane &other) const { return hitDistance < other.hitDistance; }

	void advanceOffset();
};
