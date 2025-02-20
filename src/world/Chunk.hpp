#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <array>
#include <vector>
#include <cstdint>
#include <mutex>
#include "ChunkConstants.hpp"
#include "ChunkSection.hpp"

namespace world {

class Chunk {
public:
	Chunk(int chunkX, int chunkZ);
	~Chunk();

	// Coordonnées du chunk dans le monde (en "chunks")
	int getX() const;
	int getZ() const;

	// Accède au bloc aux coordonnées locales du chunk.
	// x ∈ [0, CHUNK_SIZE_X), y ∈ [0, CHUNK_HEIGHT), z ∈ [0, CHUNK_SIZE_Z)
	int getBlock(int x, int y, int z) const;
	// Modifie le bloc aux coordonnées locales.
	void setBlock(int x, int y, int z, int blockId);

	// Marque le chunk comme "sale" pour recalcul du mesh et/ou de la lumière.
	void markDirty();

	// Indique si ce chunk doit être déchargé.
	bool shouldUnload() const;
	void setUnload(bool unloadFlag);

	// Génère le mesh pour ce chunk en combinant les meshes de chaque section.
	void generateMesh();

	// Retourne le mesh combiné (liste de vertices) pour le rendu.
	const std::vector<Vertex>& getMesh() const;

	// Pour l'accès multithreadé (par exemple lors d'une génération asynchrone du mesh).
	mutable std::mutex chunkMutex;

	// **Pour la propagation de la lumière**, les sections sont rendues publiques.
	std::array<ChunkSection, NUM_CHUNK_SECTIONS> sections;

private:
	int chunkX;
	int chunkZ;
	bool dirty;  // Indique si le chunk a besoin d'être remeshé.
	bool unload; // Flag pour décharger le chunk.

	// Mesh combiné pour l'ensemble du chunk.
	std::vector<Vertex> mesh;

	// Méthode d'aide pour générer le mesh d'une section donnée.
	void meshSection(int sectionIndex, std::vector<Vertex>& vertices);
};

} // namespace world

#endif // CHUNK_HPP
