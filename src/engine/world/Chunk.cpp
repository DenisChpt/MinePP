#include "Chunk.hpp"
#include "BlockType.hpp"
#include <iostream>

Chunk::Chunk(int x, int z) : x(x), z(z) {
	generate();
}

Chunk::~Chunk() {
	for (auto block : blocks) {
		delete block;
	}
}

void Chunk::generate() {
	// Simple terrain generation with different block types
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			BlockType type;
			if (i < 5) {
				type = BlockType::Grass;
			} else if (i < 10) {
				type = BlockType::Dirt;
			} else {
				type = BlockType::Stone;
			}
			glm::vec3 position(i , 0, j); // Set position for each block
			blocks.push_back(new Block(type, position));
		}
	}
}

void Chunk::render(const Shader& shader) {
	for (size_t i = 0; i < blocks.size(); ++i) {
		blocks[i]->render(shader, glm::mat4(1.0f));
	}
}

bool Chunk::isInside(const glm::vec3& position) const {
	return position.x >= x && position.x < x + SIZE &&
		position.z >= z && position.z < z + SIZE;
}

bool Chunk::checkFall(const glm::vec3& position) const {
	for (auto block : blocks) {
		if (block->isUnder(position)) {
			if (block->checkFall(position)) {
				return true;
			}
		}
	}
	return false;
}
