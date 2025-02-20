#include "World.hpp"
#include <iostream>
#include <glad/glad.h>

World::World(int widthInChunks, int depthInChunks)
	: width(widthInChunks), depth(depthInChunks)
{
	chunks.resize(width * depth, nullptr);
	for (int z = 0; z < depth; ++z) {
		for (int x = 0; x < width; ++x) {
			chunks[z * width + x] = new world::Chunk(x, z);
			// Générer le mesh du chunk
			chunks[z * width + x]->generateMesh();
		}
	}
}

World::~World() {
	for (auto chunk : chunks) {
		delete chunk;
	}
}

world::Chunk* World::getChunk(int chunkX, int chunkZ) const {
	if (chunkX < 0 || chunkX >= width || chunkZ < 0 || chunkZ >= depth)
		return nullptr;
	return chunks[chunkZ * width + chunkX];
}

void World::update() {
	// Ici, vous pourriez déclencher la régénération des chunks modifiés,
	// lancer le culling, ou encore mettre à jour des animations
}

void World::render() const {
	for (auto chunk : chunks) {
		if (chunk)
			chunk->render();
	}
}
