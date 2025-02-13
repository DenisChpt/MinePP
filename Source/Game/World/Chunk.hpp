#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "../../Engine/ResourceManager/AtlasGenerator.hpp"
#include "BlockType.hpp"

namespace Game {
namespace World {
static const int CHUNK_SIZE   = 16;
static const int CHUNK_HEIGHT = 64; // ajuster selon tes besoins

struct Vertex {
	float x, y, z;
	float u, v;
};

class Chunk {
public:
	Chunk(int chunkX, int chunkZ);
	~Chunk();

	void setBlock(int x, int y, int z, BlockType type);
	BlockType getBlock(int x, int y, int z) const;

	// Génère le VBO pour toutes les faces visibles
	void generateMesh(const Atlas& atlas);
	void render() const;

	// Pour la position du chunk dans le monde
	int getChunkX() const { return m_chunkX; }
	int getChunkZ() const { return m_chunkZ; }

	// Utilitaire pour insérer un arbre (type "chêne simple")
	void placeTree(int xBase, int yBase, int zBase);

private:
	BlockType getBlockSafe(int x, int y, int z) const;
	glm::vec4 getUV(const Atlas& atlas, const std::string& textureID) const;
	std::string getTextureForBlock(BlockType type, int faceIdx) const;

private:
	int m_chunkX;
	int m_chunkZ;
	BlockType m_blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	GLuint m_vao, m_vbo;
	GLsizei m_vertexCount;
};
}
} // namespace Game
