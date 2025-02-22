#ifndef CHUNKSECTION_HPP
#define CHUNKSECTION_HPP

#include <array>
#include <vector>
#include "ChunkConstants.hpp"

namespace world {

	/**
	 * @brief Represents a basic vertex with position and UV.
	 */
	struct Vertex {
		float x, y, z;
		float u, v;
	};

	/**
	 * @brief Manages a vertical subsection of a chunk, storing blocks and lighting data.
	 */
	class ChunkSection {
	public:
		ChunkSection();
		~ChunkSection();

		int getBlock(int x, int y, int z) const;
		void setBlock(int x, int y, int z, int blockId);

		unsigned char getSkyLight(int x, int y, int z) const;
		void setSkyLight(int x, int y, int z, unsigned char level);

		unsigned char getBlockLight(int x, int y, int z) const;
		void setBlockLight(int x, int y, int z, unsigned char level);

		bool dirty;
		std::vector<Vertex> meshVertices;

		std::array<int, CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z> blocks;
		std::array<unsigned char, CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z> skyLight;
		std::array<unsigned char, CHUNK_SIZE_X * CHUNK_SECTION_HEIGHT * CHUNK_SIZE_Z> blockLight;

	private:
		inline int index(int x, int y, int z) const;
	};

} // namespace world

#endif // CHUNKSECTION_HPP
