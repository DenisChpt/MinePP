#ifndef CHUNKMANAGER_HPP
#define CHUNKMANAGER_HPP

#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include "Chunk.hpp"

namespace world {

// Structure représentant les coordonnées d'un chunk.
struct ChunkCoord {
	int x, z;
	bool operator==(const ChunkCoord& other) const {
		return x == other.x && z == other.z;
	}
};

// Fonction de hachage pour ChunkCoord (utilisée dans unordered_map).
struct ChunkCoordHash {
	std::size_t operator()(const ChunkCoord& coord) const {
		std::size_t h1 = std::hash<int>()(coord.x);
		std::size_t h2 = std::hash<int>()(coord.z);
		return h1 ^ (h2 << 1);
	}
};

class ChunkManager {
public:
	ChunkManager();
	~ChunkManager();

	// Récupère le chunk aux coordonnées données, en le créant s'il n'existe pas.
	std::shared_ptr<Chunk> getChunk(int chunkX, int chunkZ);

	// Met à jour le pool de chunks en fonction de la position du joueur.
	// Les chunks dans un rayon "loadRadius" autour du joueur sont chargés,
	// tandis que les chunks trop éloignés sont marqués pour déchargement.
	void updateChunks(int playerChunkX, int playerChunkZ, int loadRadius);

	// Retourne tous les chunks (utile pour le rendu).
	std::vector<std::shared_ptr<Chunk>> getAllChunks() const;

private:
	// Pool de chunks chargés.
	std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>, ChunkCoordHash> chunkPool;
	mutable std::mutex poolMutex;
};

} // namespace world

#endif // CHUNKMANAGER_HPP
