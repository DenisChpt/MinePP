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
 * Data layout:
 *  00-08: y  coordinates    (9 bits)
 *  09-13: x  coordinates    (5 bits)
 *  14-18: z  coordinates    (5 bits)
 *  19-20: uv coordinates    (2 bits)
 *  21-28: texture index     (8 bits)
 *  29   : animation flag    (1 bit)
 *  30-31: occlusion         (2 bits)
 */


class BlockVertex
{
private:
	uint32_t data = 0;
	void setUv(bool x, bool y);
	// Remplacer setTexture par une nouvelle méthode
	// void setTexture(uint8_t x, uint8_t y); // À supprimer ou déprécier
	void setTextureIndex(uint8_t tileIndex);

public:
	BlockVertex() = default;
	BlockVertex(const glm::ivec3 &position, const glm::bvec2 &uv);

	void offset(uint32_t x, uint32_t y, uint32_t z);
	void setAnimated();
	// Mise à jour de setType pour utiliser le système dynamique
	void setType(const glm::ivec3 &offset, BlockData::BlockType type, const Assets& assets);
	[[nodiscard]] glm::ivec3 getPosition() const;
	void setOcclusionLevel(uint8_t occlusionLevel);

	static std::vector<VertexAttribute> vertexAttributes() { return {VertexAttribute(1, VertexAttribute::UInt, 0)}; }
};

static_assert(sizeof(BlockVertex) == sizeof(uint32_t), "The BlockVertex struct must not have padding");
