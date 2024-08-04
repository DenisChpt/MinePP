#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.hpp"
#include "Block.hpp"

class Chunk {
public:
	Chunk(int x, int z);
	~Chunk();

	void generate();
	void render(const Shader& shader);

	bool checkFall(const glm::vec3& position) const;
	
	int getX() const { return x; };
	int getZ() const { return z; };

	bool isInside(const glm::vec3& position) const;
	static const unsigned int SIZE = 16;

private:
	int x, z;
	std::vector<Block*> blocks;
};

#endif // CHUNK_HPP
