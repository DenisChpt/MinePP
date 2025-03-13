/**
 * @class BlockMesh
 * @brief Génère le maillage d'un bloc à partir d'un ensemble statique de sommets.
 *
 * @details La classe BlockMesh définit un tableau statique à deux dimensions de BlockVertex,
 *          chaque ligne correspondant à une face (top, bottom, east, west, north, south).
 *          La méthode getVerticesFromDirection() renvoie les sommets adaptés à la face à afficher.
 */


#pragma once

#include "BlockVertex.hpp"

struct BlockMesh
{
	static const std::array<BlockVertex, 6> &getVerticesFromDirection(const glm::ivec3 &vec)
	{
		assert(glm::abs(vec.x) + glm::abs(vec.y) + glm::abs(vec.z) == 1);

		switch (vec.x)
		{
		case 1:
			return vertices[1];
		case -1:
			return vertices[2];
		default:
			break;
		}

		switch (vec.y)
		{
		case 1:
			return vertices[0];
		case -1:
			return vertices[5];
		default:
			break;
		}

		switch (vec.z)
		{
		case 1:
			return vertices[4];
		case -1:
			return vertices[3];
		default:
			break;
		}

		assert(false);
		return vertices[0];
	}

	static const std::array<std::array<BlockVertex, 6>, 6> vertices;
};
