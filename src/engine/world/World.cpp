#include "World.hpp"

World::World() {
	generate();
}

World::~World() {
	for (auto chunk : chunks) {
		delete chunk;
	}
}

void World::generate() {
	// Simple generation of a 3x3 grid of chunks
	chunks.push_back(new Chunk(0, 0));
}

void World::render(const Shader& shader) {
	for (auto chunk : chunks) {
		chunk->render(shader);
	}
}

bool World::checkFall(const glm::vec3& position) const {
	for (auto chunk : chunks) {
		if (chunk->isInside(position)) {
			if(chunk->checkFall(position))
				return true;
		}
	}
	return false;
}
