/**
 * @class BlockVertex
 * @brief Représente un sommet d'un bloc avec position, coordonnées UV, index de texture, animation et niveau d'occlusion.
 *
 * @details La structure de données est codée sur 32 bits avec des segments dédiés à la position, aux UV, au tile index,
 *          au flag d'animation et à l'occlusion. Les méthodes setType(), setOcclusionLevel(), etc. permettent d'en modifier les valeurs.
 */


#pragma once

#include "../World/BlockTypes.hpp"
#include "../Common.hpp"
#include "Buffers.hpp"

class Assets;

/**
 * Data layout optimisé (24 bits = 3 octets):
 * 
 * uint16_t posXZ_UV (16 bits):
 *   00-04: x coordinates (5 bits) - max 31
 *   05-09: z coordinates (5 bits) - max 31  
 *   10-10: u coordinate  (1 bit)
 *   11-11: v coordinate  (1 bit)
 *   12-15: spare         (4 bits) - reserved for future use
 * 
 * uint8_t posY (8 bits):
 *   00-07: y coordinates (8 bits) - max 255
 * 
 * uint8_t textureIndex (8 bits):
 *   00-07: texture index (8 bits) - max 255
 * 
 * uint16_t flags (16 bits):
 *   00-01: occlusion     (2 bits)
 *   02-02: animated      (1 bit)
 *   03-15: spare         (13 bits) - reserved for future use
 */

#pragma pack(push, 1)
class BlockVertex
{
private:
	// Use byte array to ensure consistent memory layout
	uint8_t data[6] = {0, 0, 0, 0, 0, 0};
	
	// Layout:
	// data[0] = x(5 bits) + z_low(3 bits)
	// data[1] = z_high(2 bits) + u(1 bit) + v(1 bit) + spare(4 bits)
	// data[2] = y(8 bits)
	// data[3] = textureIndex(8 bits)
	// data[4] = occlusion(2 bits) + animated(1 bit) + spare(5 bits)
	// data[5] = spare(8 bits)

	void setUv(bool u, bool v);
	void setTextureIndexInternal(uint8_t tileIndex);

public:
	BlockVertex() = default;
	BlockVertex(const glm::ivec3 &position, const glm::bvec2 &uv);

	void setPosition(uint8_t x, uint8_t y, uint8_t z);
	void offset(uint32_t x, uint32_t y, uint32_t z);
	void setAnimated();
	void setType(const glm::ivec3 &offset, BlockData::BlockType type, const Assets& assets);
	[[nodiscard]] glm::ivec3 getPosition() const;
	void setOcclusionLevel(uint8_t occlusionLevel);

	static std::vector<VertexAttribute> vertexAttributes() { 
		// Send as 3 uint16_t for shader compatibility
		return {
			VertexAttribute(1, VertexAttribute::UShort, 0),  // bytes 0-1
			VertexAttribute(1, VertexAttribute::UShort, 2),  // bytes 2-3
			VertexAttribute(1, VertexAttribute::UShort, 4)   // bytes 4-5
		};
	}
};
#pragma pack(pop)

static_assert(sizeof(BlockVertex) == 6, "BlockVertex must be exactly 6 bytes");
