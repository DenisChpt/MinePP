#ifndef CHUNKSECTION_HPP
#define CHUNKSECTION_HPP

#include <array>
#include <vector>
#include "ChunkConstants.hpp"

namespace world {

// Structure simple pour représenter un vertex (position + UV) du mesh.
struct Vertex {
	float x, y, z;
	float u, v;
};

class ChunkSection {
public:
	ChunkSection();
	~ChunkSection();

	// Accède au bloc aux coordonnées locales dans la section.
	// x ∈ [0, CHUNK_SIZE_X), y ∈ [0, CHUNK_SECTION_HEIGHT), z ∈ [0, CHUNK_SIZE_Z)
	int getBlock(int x, int y, int z) const;
	// Définit le bloc aux coordonnées locales.
	void setBlock(int x, int y, int z, int blockId);

	// --- Fonctions pour la lumière ---

	// Accès à la lumière du ciel (skylight) à des coordonnées locales.
	unsigned char getSkyLight(int x, int y, int z) const;
	void setSkyLight(int x, int y, int z, unsigned char level);

	// Accès à la lumière émise par les blocs (block light).
	unsigned char getBlockLight(int x, int y, int z) const;
	void setBlockLight(int x, int y, int z, unsigned char level);

	// Indique que cette section a été modifiée (pour recalcul du mesh ou de la lumière).
	bool dirty;

	// Mesh généré pour cette section (utilisé par le meshing).
	std::vector<Vertex> meshVertices;

	// **IMPORTANT** : Pour faciliter la propagation de la lumière, nous conservons
	// les tableaux de données en accès direct.
	std::array<int, CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z> blocks;
	std::array<unsigned char, CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z> skyLight;
	std::array<unsigned char, CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z> blockLight;

private:
	// Convertit des coordonnées 3D locales en indice dans le tableau 1D.
	inline int index(int x, int y, int z) const;
};

} // namespace world

#endif // CHUNKSECTION_HPP
