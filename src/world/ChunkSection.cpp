#include "ChunkSection.hpp"
#include <algorithm>

namespace world {

ChunkSection::ChunkSection() : dirty(true) {
	blocks.fill(0);
	skyLight.fill(0);
	blockLight.fill(0);
}

ChunkSection::~ChunkSection() {
}

int ChunkSection::index(int x, int y, int z) const {
	return (y * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;
}

int ChunkSection::getBlock(int x, int y, int z) const {
	return blocks[index(x, y, z)];
}

void ChunkSection::setBlock(int x, int y, int z, int blockId) {
	blocks[index(x, y, z)] = blockId;
	dirty = true;
}

unsigned char ChunkSection::getSkyLight(int x, int y, int z) const {
	return skyLight[index(x, y, z)];
}

void ChunkSection::setSkyLight(int x, int y, int z, unsigned char level) {
	skyLight[index(x, y, z)] = level;
}

unsigned char ChunkSection::getBlockLight(int x, int y, int z) const {
	return blockLight[index(x, y, z)];
}

void ChunkSection::setBlockLight(int x, int y, int z, unsigned char level) {
	blockLight[index(x, y, z)] = level;
}

} // namespace world
