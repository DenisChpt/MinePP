/**
 * @class WorldGenerator
 * @brief Génère le contenu d'un chunk à l'aide d'un bruit procédural.
 *
 * @details Utilisant la bibliothèque FastNoiseLite, WorldGenerator génère des hauteurs et attribue
 * des types de blocs (stone, dirt, grass, water, bedrock) en fonction des valeurs de bruit. La
 * graine (seed) permet de garantir la reproductibilité de la génération.
 *
 * @param seed Graine pour la génération procédurale.
 */

#pragma once

#include "../Common.hpp"
#include "Chunk.hpp"

class WorldGenerator {
	int32_t seed;
	FastNoiseLite noise;

   public:
	WorldGenerator(int32_t seed);
	[[nodiscard]] int32_t getSeed() const { return seed; };

	void populateChunk(const Ref<Chunk>& chunkRef);
};