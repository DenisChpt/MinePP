#include "LightEngine.hpp"
#include "ChunkSection.hpp"
#include "Chunk.hpp"
#include "ChunkManager.hpp"
#include "ChunkConstants.hpp"
#include <algorithm>
#include <cmath>

namespace world {

LightEngine::LightEngine(ChunkManager* chunkManager)
	: chunkManager(chunkManager) {}

LightEngine::~LightEngine() {}

// --- Fonction d'assistance : conversion des coordonnées globales ---

bool LightEngine::getChunkSectionAt(int globalX, int globalY, int globalZ,
									std::shared_ptr<Chunk>& outChunk,
									int& localX, int& localY, int& localZ, int& sectionIndex) {
	// Calcul des coordonnées de chunk
	int chunkX = (globalX >= 0) ? globalX / CHUNK_SIZE_X : ((globalX + 1) / CHUNK_SIZE_X - 1);
	int chunkZ = (globalZ >= 0) ? globalZ / CHUNK_SIZE_Z : ((globalZ + 1) / CHUNK_SIZE_Z - 1);
	outChunk = chunkManager->getChunk(chunkX, chunkZ);
	if (!outChunk)
		return false;

	// Coordonnées locales dans le chunk
	localX = (globalX % CHUNK_SIZE_X + CHUNK_SIZE_X) % CHUNK_SIZE_X;
	localZ = (globalZ % CHUNK_SIZE_Z + CHUNK_SIZE_Z) % CHUNK_SIZE_Z;
	if (globalY < 0 || globalY >= CHUNK_HEIGHT)
		return false;
	sectionIndex = globalY / CHUNK_SECTION_HEIGHT;
	localY = globalY % CHUNK_SECTION_HEIGHT;
	return true;
}

// --- Fonctions d'assistance sur les propriétés des blocs ---

bool LightEngine::isTransparent(int blockId) const {
	// Ici, on considère que le bloc 0 (air) est transparent.
	// Vous pouvez étendre cette fonction pour d'autres blocs transparents.
	return (blockId == 0);
}

unsigned char LightEngine::getBlockLightEmission(int blockId) const {
	// Exemple : le bloc d'identifiant 50 (par exemple, une torche) émet une lumière de 14.
	if (blockId == 50) {
		return 14;
	}
	// Ajouter d'autres cas si nécessaire.
	return 0;
}

// --- Mise à jour globale de la lumière ---

void LightEngine::updateLighting() {
	// On met à jour d'abord le skylight, puis la block light.
	propagateSkyLight();
	propagateBlockLight();
}

// --- Propagation du skylight ---

void LightEngine::propagateSkyLight() {
	std::queue<LightNode> queue;

	// Lambda pour obtenir l'identifiant d'un bloc à partir de coordonnées globales.
	auto getBlockGlobal = [this](int gx, int gy, int gz) -> int {
		std::shared_ptr<Chunk> chunk;
		int lx, ly, lz, sectionIndex;
		if (!getChunkSectionAt(gx, gy, gz, chunk, lx, ly, lz, sectionIndex))
			return 0;
		return chunk->sections[sectionIndex].getBlock(lx, ly, lz);
	};

	// Réinitialisation de la lumière du ciel dans tous les chunks chargés.
	auto allChunks = chunkManager->getAllChunks();
	for (auto& chunk : allChunks) {
		std::lock_guard<std::mutex> lock(chunk->chunkMutex);
		for (int sectionIndex = 0; sectionIndex < NUM_CHUNK_SECTIONS; ++sectionIndex) {
			ChunkSection& section = chunk->sections[sectionIndex];
			int totalBlocks = CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z;
			for (int i = 0; i < totalBlocks; ++i) {
				section.skyLight[i] = 0;
			}
		}
	}

	// Initialisation du skylight : pour chaque colonne (global x,z),
	// on part du haut du monde et on assigne la lumière maximale (15) tant que le bloc est transparent.
	for (auto& chunk : allChunks) {
		std::lock_guard<std::mutex> lock(chunk->chunkMutex);
		int baseX = chunk->getX() * CHUNK_SIZE_X;
		int baseZ = chunk->getZ() * CHUNK_SIZE_Z;
		for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
			for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
				int globalX = baseX + lx;
				int globalZ = baseZ + lz;
				int globalY = CHUNK_HEIGHT - 1;
				while (globalY >= 0) {
					std::shared_ptr<Chunk> curChunk;
					int localX, localY, localZ, sectionIndex;
					if (!getChunkSectionAt(globalX, globalY, globalZ, curChunk, localX, localY, localZ, sectionIndex))
						break;
					int blockId = curChunk->sections[sectionIndex].getBlock(localX, localY, localZ);
					if (!isTransparent(blockId))
						break; // Arrêt dès qu'un bloc opaque est rencontré.
					curChunk->sections[sectionIndex].setSkyLight(localX, localY, localZ, 15);
					queue.push({globalX, globalY, globalZ, 15});
					--globalY;
				}
			}
		}
	}

	// Propagation BFS du skylight.
	while (!queue.empty()) {
		LightNode node = queue.front();
		queue.pop();
		unsigned char currentLight = node.light;
		if (currentLight <= 1)
			continue; // Plus rien à propager.
		const int offsets[6][3] = {
			{1, 0, 0}, {-1, 0, 0},
			{0, 1, 0}, {0, -1, 0},
			{0, 0, 1}, {0, 0, -1}
		};

		for (int i = 0; i < 6; ++i) {
			int nx = node.x + offsets[i][0];
			int ny = node.y + offsets[i][1];
			int nz = node.z + offsets[i][2];
			std::shared_ptr<Chunk> neighborChunk;
			int localX, localY, localZ, sectionIndex;
			if (!getChunkSectionAt(nx, ny, nz, neighborChunk, localX, localY, localZ, sectionIndex))
				continue;
			int neighborBlock = getBlockGlobal(nx, ny, nz);
			if (!isTransparent(neighborBlock))
				continue;
			unsigned char neighborLight = neighborChunk->sections[sectionIndex].getSkyLight(localX, localY, localZ);
			unsigned char propagatedLight = currentLight - 1;
			if (neighborLight < propagatedLight) {
				neighborChunk->sections[sectionIndex].setSkyLight(localX, localY, localZ, propagatedLight);
				queue.push({nx, ny, nz, propagatedLight});
			}
		}
	}
}

// --- Propagation de la block light ---

void LightEngine::propagateBlockLight() {
	std::queue<LightNode> queue;
	auto getBlockGlobal = [this](int gx, int gy, int gz) -> int {
		std::shared_ptr<Chunk> chunk;
		int lx, ly, lz, sectionIndex;
		if (!getChunkSectionAt(gx, gy, gz, chunk, lx, ly, lz, sectionIndex))
			return 0;
		return chunk->sections[sectionIndex].getBlock(lx, ly, lz);
	};

	auto allChunks = chunkManager->getAllChunks();
	for (auto& chunk : allChunks) {
		std::lock_guard<std::mutex> lock(chunk->chunkMutex);
		for (int sectionIndex = 0; sectionIndex < NUM_CHUNK_SECTIONS; ++sectionIndex) {
			ChunkSection& section = chunk->sections[sectionIndex];
			int totalBlocks = CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z;
			for (int i = 0; i < totalBlocks; ++i) {
				section.blockLight[i] = 0;
			}
		}
	}

	// Initialisation de la block light à partir des sources lumineuses.
	for (auto& chunk : allChunks) {
		std::lock_guard<std::mutex> lock(chunk->chunkMutex);
		int baseX = chunk->getX() * CHUNK_SIZE_X;
		int baseZ = chunk->getZ() * CHUNK_SIZE_Z;
		for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
			for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
				for (int ly = 0; ly < CHUNK_HEIGHT; ++ly) {
					int blockId = chunk->getBlock(lx, ly, lz);
					unsigned char emission = getBlockLightEmission(blockId);
					if (emission > 0) {
						int globalX = baseX + lx;
						int globalY = ly;
						int globalZ = baseZ + lz;
						std::shared_ptr<Chunk> curChunk;
						int localX, localY, localZ, sectionIndex;
						if (!getChunkSectionAt(globalX, globalY, globalZ, curChunk, localX, localY, localZ, sectionIndex))
							continue;
						curChunk->sections[sectionIndex].setBlockLight(localX, localY, localZ, emission);
						queue.push({globalX, globalY, globalZ, emission});
					}
				}
			}
		}
	}

	// Propagation BFS de la block light.
	while (!queue.empty()) {
		LightNode node = queue.front();
		queue.pop();
		unsigned char currentLight = node.light;
		if (currentLight <= 1)
			continue;
		const int offsets[6][3] = {
			{1, 0, 0}, {-1, 0, 0},
			{0, 1, 0}, {0, -1, 0},
			{0, 0, 1}, {0, 0, -1}
		};

		for (int i = 0; i < 6; ++i) {
			int nx = node.x + offsets[i][0];
			int ny = node.y + offsets[i][1];
			int nz = node.z + offsets[i][2];
			std::shared_ptr<Chunk> neighborChunk;
			int localX, localY, localZ, sectionIndex;
			if (!getChunkSectionAt(nx, ny, nz, neighborChunk, localX, localY, localZ, sectionIndex))
				continue;
			int neighborBlock = getBlockGlobal(nx, ny, nz);
			if (!isTransparent(neighborBlock))
				continue;
			unsigned char neighborLight = neighborChunk->sections[sectionIndex].getBlockLight(localX, localY, localZ);
			unsigned char propagatedLight = currentLight - 1;
			if (neighborLight < propagatedLight) {
				neighborChunk->sections[sectionIndex].setBlockLight(localX, localY, localZ, propagatedLight);
				queue.push({nx, ny, nz, propagatedLight});
			}
		}
	}
}

} // namespace world
