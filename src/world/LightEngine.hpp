#ifndef LIGHTENGINE_HPP
#define LIGHTENGINE_HPP

#include <queue>
#include <tuple>
#include <memory>
#include "ChunkManager.hpp"

namespace world {

	/**
	 * @brief Handles sky and block lighting propagation throughout the loaded world.
	 */
	class LightEngine {
	public:
		LightEngine(ChunkManager* chunkManager);
		~LightEngine();

		/**
		 * @brief Rebuilds both skylight and block light for all chunks.
		 */
		void updateLighting();

	private:
		ChunkManager* chunkManager;

		void propagateSkyLight();
		void propagateBlockLight();

		bool getChunkSectionAt(int globalX, int globalY, int globalZ,
							   std::shared_ptr<Chunk>& outChunk,
							   int& localX, int& localY, int& localZ, int& sectionIndex);

		bool isTransparent(int blockId) const;
		unsigned char getBlockLightEmission(int blockId) const;

		struct LightNode {
			int x, y, z;
			unsigned char light;
		};
	};

} // namespace world

#endif // LIGHTENGINE_HPP
