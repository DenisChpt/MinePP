/**
 * @class WorldRayCast
 * @brief Effectue un lancer de rayon dans le monde pour détecter les blocs.
 *
 * @details La classe WorldRayCast lance un rayon depuis une position dans une direction donnée afin de déterminer
 *          quel bloc est touché (hitTarget). Elle s'appuie sur des instances d'AxisPlane pour calculer les intersections
 *          avec les plans des blocs.
 *
 * @param position Position de départ du rayon.
 * @param direction Direction du rayon.
 * @param world Référence au monde dans lequel le lancer est effectué.
 * @param reach Distance maximale du lancer.
 *
 * @return Un booléen indiquant si un bloc a été touché et, via getHitTarget(), les informations sur le bloc.
 */


#pragma once

#include "../World/World.hpp"
#include "../MinePP.hpp"

struct HitTarget
{
	glm::vec3 position;
	BlockData block;
	glm::vec3 neighbor;
	bool hasNeighbor = false;
};

class WorldRayCast
{
	bool successful;
	HitTarget hitTarget;

public:
	WorldRayCast(glm::vec3 position, glm::vec3 direction, World &world, float reach);

	[[nodiscard]] bool hasHit() const { return successful; };
	[[nodiscard]] HitTarget getHitTarget() const { return hitTarget; };
	explicit operator bool() const { return hasHit(); }
};
