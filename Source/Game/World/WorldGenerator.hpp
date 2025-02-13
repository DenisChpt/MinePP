#pragma once

#include <vector>
#include <memory>
#include "Chunk.hpp"
#include "../../Engine/ResourceManager/AtlasGenerator.hpp" // si besoin pour la struct Atlas

namespace Game {

class WorldGenerator {
public:
	WorldGenerator(unsigned int seed = 0);
	~WorldGenerator();

	/**
	 * @brief Génère un tableau de chunks, chacun contenant des blocs.
	 * @param atlas L'atlas de textures pour associer chaque bloc à sa texture.
	 */
	std::vector<std::shared_ptr<World::Chunk>> generateWorld(const Atlas& atlas);

private:
	unsigned int m_seed;

	float perlin2D(float x, float y, int repeat = 256) const;
	float perlinFractal2D(float x, float y, int octaves, float persistence, float frequency) const;
};

} // namespace Game
