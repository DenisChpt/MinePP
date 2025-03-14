/**
 * @struct AABB
 * @brief Représente une boîte englobante alignée sur les axes (Axis-Aligned Bounding Box).
 *
 * @details La structure AABB définit les points minimum et maximum et offre des méthodes pour tester l'intersection
 *          avec une autre AABB. Une méthode statique fromBlockPosition() permet de créer une AABB pour un bloc de taille unité.
 *
 * @return true si les boîtes se chevauchent, false sinon.
 */


#pragma once

#include "../MinePP.hpp"

struct AABB
{
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

	explicit constexpr AABB(const glm::vec3 &minPoint, const glm::vec3 &maxPoint)
		: minPoint(minPoint),
		  maxPoint(maxPoint) {}

	static AABB fromBlockPosition(const glm::vec3 &position) { return AABB(position, position + glm::vec3{1, 1, 1}); }

	[[nodiscard]] bool intersect(const AABB &aabb) const
	{
		return (getMinX() <= aabb.getMaxX() && getMaxX() >= aabb.getMinX()) &&
			   (getMinY() <= aabb.getMaxY() && getMaxY() >= aabb.getMinY()) &&
			   (getMinZ() <= aabb.getMaxZ() && getMaxZ() >= aabb.getMinZ());
	}

	[[nodiscard]] glm::vec3 getMinPoint() const { return minPoint; };
	[[nodiscard]] glm::vec3 getMaxPoint() const { return maxPoint; };

	[[nodiscard]] float getMinX() const { return minPoint.x; };
	[[nodiscard]] float getMinY() const { return minPoint.y; };
	[[nodiscard]] float getMinZ() const { return minPoint.z; };
	[[nodiscard]] float getMaxX() const { return maxPoint.x; };
	[[nodiscard]] float getMaxY() const { return maxPoint.y; };
	[[nodiscard]] float getMaxZ() const { return maxPoint.z; };
};