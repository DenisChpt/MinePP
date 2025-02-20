#ifndef LIGHTENGINE_HPP
#define LIGHTENGINE_HPP

#include <queue>
#include <tuple>
#include <memory>
#include "ChunkManager.hpp"

namespace world {

class LightEngine {
public:
	// Le constructeur reçoit un pointeur vers le ChunkManager.
	LightEngine(ChunkManager* chunkManager);
	~LightEngine();

	// Recalcule la lumière (skylight et block light) pour le monde entier.
	void updateLighting();

private:
	ChunkManager* chunkManager;

	// Propagation de la lumière du ciel (skylight) par flood-fill.
	void propagateSkyLight();
	// Propagation de la lumière émise par les blocs (block light) par flood-fill.
	void propagateBlockLight();

	// Convertit des coordonnées globales (x,y,z) en chunk, section et coordonnées locales.
	// Renvoie false si le chunk n'est pas chargé.
	bool getChunkSectionAt(int globalX, int globalY, int globalZ,
						   std::shared_ptr<Chunk>& outChunk,
						   int& localX, int& localY, int& localZ, int& sectionIndex);

	// Détermine si un bloc est transparent (autorise la propagation de la lumière).
	bool isTransparent(int blockId) const;

	// Renvoie l'intensité lumineuse émise par un bloc (de 0 à 15).
	unsigned char getBlockLightEmission(int blockId) const;

	// Structure utilisée pour la propagation (BFS).
	struct LightNode {
		int x, y, z;
		unsigned char light;
	};
};

} // namespace world

#endif // LIGHTENGINE_HPP
