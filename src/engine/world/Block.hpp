#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "BlockType.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include <array>
#include <vector>

class Block {
public:
	Block(BlockType type, const glm::vec3& position);
	~Block();

	const std::array<std::array<glm::vec2, 4>, 6>& getTextureCoords() const;
	BlockType getType() const;
	void render(const Shader& shader);
	Texture* getTexture() const { return texture; }
	glm::vec3 getPosition() const { return position; }
	bool isUnder(const glm::vec3& position) const;
	bool checkFall(const glm::vec3& position) const;

private:
	glm::vec3 position;
	BlockType type;
	BlockSize size;
	Texture* texture;
	std::array<std::array<glm::vec2, 4>, 6> textureCoords;
	GLuint VAO, VBO;

	void setTextureCoords();
	void setupMesh();
};

#endif // BLOCK_HPP