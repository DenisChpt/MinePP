#ifndef WORLD_HPP
#define WORLD_HPP

#include "Chunk.hpp"
#include <vector>

/**
 * @brief Represents a simplistic World container holding multiple chunks in a grid.
 */
class World {
public:
	World(int widthInChunks, int depthInChunks);
	~World();

	world::Chunk* getChunk(int chunkX, int chunkZ) const;
	void update();

	int getWidthInChunks() const { return width; }
	int getDepthInChunks() const { return depth; }

private:
	int width;
	int depth;
	std::vector<world::Chunk*> chunks;
};

#endif // WORLD_HPP
