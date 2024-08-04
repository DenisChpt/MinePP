#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include "Chunk.hpp"
#include "Shader.hpp"

class World {
public:
	World();
	~World();

	void generate();
	void render(const Shader& shader);
	bool checkFall(const glm::vec3& position) const;

private:
	std::vector<Chunk*> chunks;
};

#endif // WORLD_HPP
