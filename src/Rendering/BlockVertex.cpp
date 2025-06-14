#include "BlockVertex.hpp"

#include "../Core/Assets.hpp"

BlockVertex::BlockVertex(const glm::ivec3& position, const glm::bvec2& uv) {
	setPosition(position.x, position.y, position.z);
	setUv(uv.x, uv.y);
}

void BlockVertex::setPosition(uint8_t x, uint8_t y, uint8_t z) {
	assert(x <= 31 && "X coordinate must be <= 31");
	assert(z <= 31 && "Z coordinate must be <= 31");
	assert(y <= 255 && "Y coordinate must be <= 255");

	// data[0] = x(5 bits) + z_low(3 bits)
	data[0] = (x & 0x1F) | ((z & 0x07) << 5);

	// data[1] = z_high(2 bits) + u(1 bit) + v(1 bit) + spare(4 bits)
	data[1] = (data[1] & 0xFC) | ((z >> 3) & 0x03);

	// data[2] = y(8 bits)
	data[2] = y;
}

void BlockVertex::setUv(bool u, bool v) {
	// UV bits are in data[1] at positions 2-3
	data[1] = (data[1] & 0xF3) | (u ? (1 << 2) : 0) | (v ? (1 << 3) : 0);
}

void BlockVertex::setTextureIndexInternal(uint8_t tileIndex) {
	// Texture index is data[3]
	data[3] = tileIndex;
}

void BlockVertex::offset(uint32_t x, uint32_t y, uint32_t z) {
	// Extract current position
	uint8_t currentX = data[0] & 0x1F;
	uint8_t currentZ = ((data[0] >> 5) & 0x07) | ((data[1] & 0x03) << 3);
	uint8_t currentY = data[2];

	// Add offset
	uint8_t newX = currentX + x;
	uint8_t newY = currentY + y;
	uint8_t newZ = currentZ + z;

	// Validate new position
	assert(newX <= 31 && "X coordinate out of bounds");
	assert(newZ <= 31 && "Z coordinate out of bounds");
	assert(newY <= 255 && "Y coordinate out of bounds");

	// Set new position
	setPosition(newX, newY, newZ);
}

void BlockVertex::setAnimated() {
	// Animated flag is bit 2 in data[4]
	data[4] |= (1 << 2);
}

glm::ivec3 BlockVertex::getPosition() const {
	uint8_t x = data[0] & 0x1F;
	uint8_t z = ((data[0] >> 5) & 0x07) | ((data[1] & 0x03) << 3);
	uint8_t y = data[2];
	return glm::ivec3(x, y, z);
}

void BlockVertex::setOcclusionLevel(uint8_t occlusionLevel) {
	assert(occlusionLevel < 4 && "Occlusion level must be < 4");
	// Occlusion bits are 0-1 in data[4]
	data[4] = (data[4] & 0xFC) | (occlusionLevel & 0x03);
}

void BlockVertex::setType(const glm::ivec3& offset,
						  BlockData::BlockType type,
						  const Assets& assets) {
	// Détermine l'index de face en fonction de l'offset
	// Convention : 0 = top, 1 = east, 2 = west, 3 = north, 4 = south, 5 = bottom
	int faceIndex = 0;
	if (offset.y == 1) {
		faceIndex = 0;	// top
	} else if (offset.y == -1) {
		faceIndex = 5;	// bottom
	} else if (offset.x == 1) {
		faceIndex = 1;	// east
	} else if (offset.x == -1) {
		faceIndex = 2;	// west
	} else if (offset.z == 1) {
		faceIndex = 4;	// south
	} else if (offset.z == -1) {
		faceIndex = 3;	// north
	}
	// Récupère la configuration de texture pour ce type de bloc via Assets
	BlockTextureData btd = assets.getBlockTextureData(type);
	// Stocke l'index correspondant à la face dans le vertex
	uint8_t textureIdx = btd.faceIndices[faceIndex];
	setTextureIndexInternal(textureIdx);
}