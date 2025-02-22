#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <array>
#include <vector>
#include <cstdint>
#include <mutex>
#include "ChunkConstants.hpp"
#include "ChunkSection.hpp"

namespace world {

	/**
	 * @brief Represents a single chunk composed of multiple chunk sections stacked vertically.
	 */
	class Chunk {
	public:
		Chunk(int chunkX, int chunkZ);
		~Chunk();

		int getX() const;
		int getZ() const;

		int getBlock(int x, int y, int z) const;
		void setBlock(int x, int y, int z, int blockId);

		void markDirty();

		bool shouldUnload() const;
		void setUnload(bool unloadFlag);

		/**
		 * @brief Generates the mesh for this chunk by combining section meshes.
		 */
		void generateMesh();

		/**
		 * @brief Retrieves the final set of vertices for rendering.
		 */
		const std::vector<Vertex>& getMesh() const;

		mutable std::mutex chunkMutex;
		std::array<ChunkSection, NUM_CHUNK_SECTIONS> sections;

	private:
		int chunkX;
		int chunkZ;
		bool dirty;
		bool unload;

		std::vector<Vertex> mesh;

		void meshSection(int sectionIndex, std::vector<Vertex>& vertices);
	};

} // namespace world

#endif // CHUNK_HPP
