#ifndef CHUNKMANAGER_HPP
#define CHUNKMANAGER_HPP

#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include "Chunk.hpp"

namespace world {

	struct ChunkCoord {
		int x, z;
		bool operator==(const ChunkCoord& other) const {
			return x == other.x && z == other.z;
		}
	};

	struct ChunkCoordHash {
		std::size_t operator()(const ChunkCoord& coord) const {
			std::size_t h1 = std::hash<int>()(coord.x);
			std::size_t h2 = std::hash<int>()(coord.z);
			return h1 ^ (h2 << 1);
		}
	};

	/**
	 * @brief Manages the loading and storage of chunks in the world.
	 */
	class ChunkManager {
	public:
		ChunkManager();
		~ChunkManager();

		/**
		 * @brief Retrieves a chunk at a given chunk coordinate, creating it if necessary.
		 */
		std::shared_ptr<Chunk> getChunk(int chunkX, int chunkZ);

		/**
		 * @brief Updates which chunks should be loaded or unloaded based on player position and load radius.
		 */
		void updateChunks(int playerChunkX, int playerChunkZ, int loadRadius);

		/**
		 * @brief Retrieves all chunks currently stored.
		 */
		std::vector<std::shared_ptr<Chunk>> getAllChunks() const;

	private:
		std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>, ChunkCoordHash> chunkPool;
		mutable std::mutex poolMutex;
	};

} // namespace world

#endif // CHUNKMANAGER_HPP
