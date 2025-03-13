/**
 * @class Util
 * @brief Classe utilitaire regroupant diverses fonctions d'aide.
 *
 * @details La classe Util fournit notamment une méthode readBinaryFile() pour lire un fichier en mode binaire,
 *          positiveMod() pour calculer un modulo positif, ainsi que des classes de hachage (HashVec2, HashVec3) et
 *          de comparaison (CompareIVec3) pour les types glm::ivec2 et glm::ivec3.
 */


#pragma once

#include "../MinePP.hpp"

class Util
{
	Util() = default;

public:
	class HashVec2
	{
	public:
		size_t operator()(const glm::ivec2 &coord) const noexcept
		{
			size_t hash = coord.x;
			hash ^= coord.y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};
	class HashVec3
	{
	public:
		size_t operator()(const glm::ivec3 &coord) const noexcept
		{
			size_t hash = coord.x;
			hash ^= coord.y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= coord.z + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		};
	};

	class CompareIVec3
	{
	public:
		int operator()(const glm::ivec3 &lhs, const glm::ivec3 &rhs) const
		{
			if (lhs.x < rhs.x)
				return true;
			if (lhs.x == rhs.x)
			{
				if (lhs.y < rhs.y)
					return true;

				if (lhs.y == rhs.y)
				{
					return lhs.z < rhs.z;
				}
				return false;
			}
			return false;
		}
	};

	static Ref<std::string> readBinaryFile(const std::string &path);
	static int32_t positiveMod(int32_t num, int32_t divisor);
};