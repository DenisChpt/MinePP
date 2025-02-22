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
	if (chunkX < 0 || chunkX >= width || chunkZ < 0 || chunkZ >= depth) {
		return nullptr;
	}
	return chunks[chunkZ * width + chunkX];
}

void World::update() {
	// Logic to update world state or trigger chunk regeneration if needed
}
