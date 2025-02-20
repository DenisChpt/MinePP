#ifndef WORLD_HPP
#define WORLD_HPP

#include "Chunk.hpp"

class World {
public:
	World(int widthInChunks, int depthInChunks);
	~World();

	// Renvoie le chunk situé aux coordonnées données (en unités de chunk)
	world::Chunk* getChunk(int chunkX, int chunkZ) const;

	// Met à jour le monde (par exemple, génération ou mises à jour de chunks)
	void update();

	// Rendu de tous les chunks
	// void render() const;

	int getWidthInChunks() const { return width; }
	int getDepthInChunks() const { return depth; }

private:
	int width;
	int depth;
	std::vector<world::Chunk*> chunks;
};

#endif // WORLD_HPP
