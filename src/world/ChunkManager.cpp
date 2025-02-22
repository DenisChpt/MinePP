#include "ChunkManager.hpp"
#include <cstdlib>

namespace world {

ChunkManager::ChunkManager() {
}

ChunkManager::~ChunkManager() {
}

std::shared_ptr<Chunk> ChunkManager::getChunk(int chunkX, int chunkZ) {
	std::lock_guard<std::mutex> lock(poolMutex);
	ChunkCoord coord { chunkX, chunkZ };
	auto it = chunkPool.find(coord);
	if (it != chunkPool.end()) {
		return it->second;
	}
	std::shared_ptr<Chunk> newChunk = std::make_shared<Chunk>(chunkX, chunkZ);
	chunkPool[coord] = newChunk;
	return newChunk;
}

void ChunkManager::updateChunks(int playerChunkX, int playerChunkZ, int loadRadius) {
	std::lock_guard<std::mutex> lock(poolMutex);

	for (auto& pair : chunkPool) {
		int dx = pair.first.x - playerChunkX;
		int dz = pair.first.z - playerChunkZ;
		if (abs(dx) > loadRadius || abs(dz) > loadRadius) {
			pair.second->setUnload(true);
		} else {
			pair.second->setUnload(false);
		}
	}

	for (int x = playerChunkX - loadRadius; x <= playerChunkX + loadRadius; ++x) {
		for (int z = playerChunkZ - loadRadius; z <= playerChunkZ + loadRadius; ++z) {
			ChunkCoord coord { x, z };
			if (chunkPool.find(coord) == chunkPool.end()) {
				chunkPool[coord] = std::make_shared<Chunk>(x, z);
			}
		}
	}
}

std::vector<std::shared_ptr<Chunk>> ChunkManager::getAllChunks() const {
	std::lock_guard<std::mutex> lock(poolMutex);
	std::vector<std::shared_ptr<Chunk>> chunks;
	for (const auto& pair : chunkPool) {
		chunks.push_back(pair.second);
	}
	return chunks;
}

} // namespace world
