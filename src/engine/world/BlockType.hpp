#ifndef BLOCKTYPE_HPP
#define BLOCKTYPE_HPP

#include <unordered_map>
#include <glm/glm.hpp>

enum class BlockType {
	Grass,
	Dirt,
	Stone,
	Water
};

struct BlockSize {
	float width;
	float height;
	float depth;
};

inline BlockSize getBlockSize(BlockType type) {
	static std::unordered_map<BlockType, BlockSize> blockSizes = {
		{ BlockType::Grass, {16.0f, 16.0f, 16.0f} },
		{ BlockType::Dirt,  {16.0f, 16.0f, 16.0f} },
		{ BlockType::Stone, {16.0f, 16.0f, 16.0f} },
		{ BlockType::Water, {16.0f, 16.0f, 16.0f} }
	};
	return blockSizes[type];
}

#endif // BLOCKTYPE_HPP
