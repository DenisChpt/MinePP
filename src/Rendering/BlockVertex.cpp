#include "BlockVertex.hpp"
#include "../AssetManager/TextureAtlas.hpp"

BlockVertex::BlockVertex(const glm::ivec3 &position, const glm::bvec2 &uv)
{
	offset(position.x, position.y, position.z);
	setUv(uv.x, uv.y);
}

void BlockVertex::setUv(bool x, bool y)
{
	uint8_t uv = x | (y << 1);
	assert((((data >> 19) & 0xff) + uv) <= 0xff && "UV Coordinates are out of bounds");

	data += uv << 19;
};

void BlockVertex::setTextureIndex(uint8_t tileIndex)
{
	// Efface les bits 20 à 27 puis insère la nouvelle valeur
	data &= ~(0xFF << 20);
	data |= (tileIndex & 0xFF) << 20;
}

void BlockVertex::offset(uint32_t x, uint32_t y, uint32_t z)
{
	assert((((data >> 9) & 0x1fu) + x) <= 16 && "Coordinate is out of bounds");
	assert((((data >> 14) & 0x1fu) + z) <= 16 && "Coordinate is out of bounds");
	assert(((data & 0x1ffu) + y) <= 256 && "Coordinate is out of bounds");

	data += y;
	data += x << 9;
	data += z << 14;
}

void BlockVertex::setAnimated()
{
	data |= 0b1 << 28;
}

glm::ivec3 BlockVertex::getPosition() const
{
	return {(data >> 9) & 0x1fu, data & 0x1ffu, (data >> 14) & 0x1fu};
}

void BlockVertex::setOcclusionLevel(uint8_t occlusionLevel)
{
	assert(occlusionLevel < 4 && "The occlusion level is out of bounds");
	data |= occlusionLevel << 29;
}

void BlockVertex::setType(const glm::ivec3 &offset, BlockData::BlockType type)
{
	// Détermine l'index de face en fonction de l'offset
	// Convention : 0 = top, 1 = east, 2 = west, 3 = north, 4 = south, 5 = bottom
	int faceIndex = 0;
	if (offset.y == 1)
	{
		faceIndex = 0; // top
	}
	else if (offset.y == -1)
	{
		faceIndex = 5; // bottom
	}
	else if (offset.x == 1)
	{
		faceIndex = 1; // east
	}
	else if (offset.x == -1)
	{
		faceIndex = 2; // west
	}
	else if (offset.z == 1)
	{
		faceIndex = 4; // south
	}
	else if (offset.z == -1)
	{
		faceIndex = 3; // north
	}
	// Récupère la configuration de texture pour ce type de bloc via TextureAtlas
	BlockTextureData btd = TextureAtlas::instance().getBlockTextureData(type);
	// Stocke l'index correspondant à la face dans le vertex
	setTextureIndex(btd.faceIndices[faceIndex]);
}
