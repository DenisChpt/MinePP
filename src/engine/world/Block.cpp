#include "Block.hpp"
#include <iostream>

Block::Block(BlockType type, const glm::vec3& position) : type(type), position(position), size(getBlockSize(type))
{
	texture = new Texture("../assets/textures/atlas.png");
	setTextureCoords();
	setupMesh();
}

Block::~Block()
{
	delete texture;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

BlockType Block::getType() const
{
	return type;
}
const std::array<std::array<glm::vec2, 4>, 6> &Block::getTextureCoords() const
{
	return textureCoords;
}

void Block::setTextureCoords() {

	int row = static_cast<int>(type); // Assuming BlockType enums are ordered correctly

	float unitWidth = 1.0f / texture->getWidth();
	float unitHeight = 1.0f / texture->getHeight();

	float topY = row * size.height * unitHeight;
	float bottomY = (row + 1) * size.height * unitHeight;

	textureCoords = {
		// Top face
		std::array<glm::vec2, 4>{
			glm::vec2(0 * size.width * unitWidth, topY),                // Top-left
			glm::vec2(1 * size.width * unitWidth, topY),                // Top-right
			glm::vec2(1 * size.width * unitWidth, bottomY),             // Bottom-right
			glm::vec2(0 * size.width * unitWidth, bottomY)             // Bottom-left
		},

		// Bottom face
		std::array<glm::vec2, 4>{
			glm::vec2(1 * size.width * unitWidth, topY),                // Top-left
			glm::vec2(2 * size.width * unitWidth, topY),                // Top-right
			glm::vec2(2 * size.width * unitWidth, bottomY),             // Bottom-right
			glm::vec2(1 * size.width * unitWidth, bottomY)             // Bottom-left
		},
		
		// Front face
		std::array<glm::vec2, 4>{
			glm::vec2(2 * size.width * unitWidth, topY),                // Top-left
			glm::vec2(3 * size.width * unitWidth, topY),                // Top-right
			glm::vec2(3 * size.width * unitWidth, bottomY),             // Bottom-right
			glm::vec2(2 * size.width * unitWidth, bottomY)             // Bottom-left
		},

		// Back face
		std::array<glm::vec2, 4>{
			glm::vec2(3 * size.width * unitWidth, topY),                // Top-left
			glm::vec2(4 * size.width * unitWidth, topY),                // Top-right
			glm::vec2(4 * size.width * unitWidth, bottomY),             // Bottom-right
			glm::vec2(3 * size.width * unitWidth, bottomY)             // Bottom-left
		},

		// Left face
		std::array<glm::vec2, 4>{
			glm::vec2(4 * size.width * unitWidth, topY),                // Top-left
			glm::vec2(5 * size.width * unitWidth, topY),                // Top-right
			glm::vec2(5 * size.width * unitWidth, bottomY),             // Bottom-right
			glm::vec2(4 * size.width * unitWidth, bottomY)              // Bottom-left
		},

		// Right face
		std::array<glm::vec2, 4>{
			glm::vec2(5 * size.width * unitWidth, topY),                // Top-left
			glm::vec2(6 * size.width * unitWidth, topY),                // Top-right
			glm::vec2(6 * size.width * unitWidth, bottomY),             // Bottom-right
			glm::vec2(5 * size.width * unitWidth, bottomY)              // Bottom-left
		}
	};
}

void Block::setupMesh() {
	float vertices[] = {
		// Top face
		-0.5f,  0.5f, -0.5f, textureCoords[0][0].x, textureCoords[0][0].y,
		-0.5f,  0.5f,  0.5f, textureCoords[0][3].x, textureCoords[0][3].y,
		 0.5f,  0.5f,  0.5f, textureCoords[0][2].x, textureCoords[0][2].y,
		 0.5f,  0.5f,  0.5f, textureCoords[0][2].x, textureCoords[0][2].y,
		 0.5f,  0.5f, -0.5f, textureCoords[0][1].x, textureCoords[0][1].y,
		-0.5f,  0.5f, -0.5f, textureCoords[0][0].x, textureCoords[0][0].y,
		
		// Bottom face
		-0.5f, -0.5f, -0.5f, textureCoords[1][0].x, textureCoords[1][0].y,
		 0.5f, -0.5f, -0.5f, textureCoords[1][1].x, textureCoords[1][1].y,
		 0.5f, -0.5f,  0.5f, textureCoords[1][2].x, textureCoords[1][2].y,
		 0.5f, -0.5f,  0.5f, textureCoords[1][2].x, textureCoords[1][2].y,
		-0.5f, -0.5f,  0.5f, textureCoords[1][3].x, textureCoords[1][3].y,
		-0.5f, -0.5f, -0.5f, textureCoords[1][0].x, textureCoords[1][0].y,

		// Front face
		-0.5f, -0.5f,  0.5f, textureCoords[2][2].x, textureCoords[2][2].y,
		 0.5f, -0.5f,  0.5f, textureCoords[2][3].x, textureCoords[2][3].y,
		 0.5f,  0.5f,  0.5f, textureCoords[2][0].x, textureCoords[2][0].y,
		 0.5f,  0.5f,  0.5f, textureCoords[2][0].x, textureCoords[2][0].y,
		-0.5f,  0.5f,  0.5f, textureCoords[2][1].x, textureCoords[2][1].y,
		-0.5f, -0.5f,  0.5f, textureCoords[2][2].x, textureCoords[2][2].y,

		// Back face
		-0.5f, -0.5f, -0.5f, textureCoords[3][3].x, textureCoords[3][3].y,
		-0.5f,  0.5f, -0.5f, textureCoords[3][0].x, textureCoords[3][0].y,
		 0.5f,  0.5f, -0.5f, textureCoords[3][1].x, textureCoords[3][1].y,
		 0.5f,  0.5f, -0.5f, textureCoords[3][1].x, textureCoords[3][1].y,
		 0.5f, -0.5f, -0.5f, textureCoords[3][2].x, textureCoords[3][2].y,
		-0.5f, -0.5f, -0.5f, textureCoords[3][3].x, textureCoords[3][3].y,

		// Left face
		-0.5f,  0.5f,  0.5f, textureCoords[4][0].x, textureCoords[4][0].y,
		-0.5f,  0.5f, -0.5f, textureCoords[4][1].x, textureCoords[4][1].y,
		-0.5f, -0.5f, -0.5f, textureCoords[4][2].x, textureCoords[4][2].y,
		-0.5f, -0.5f, -0.5f, textureCoords[4][2].x, textureCoords[4][2].y,
		-0.5f, -0.5f,  0.5f, textureCoords[4][3].x, textureCoords[4][3].y,
		-0.5f,  0.5f,  0.5f, textureCoords[4][0].x, textureCoords[4][0].y,

		// Right face
		 0.5f,  0.5f,  0.5f, textureCoords[5][0].x, textureCoords[5][0].y,
		 0.5f, -0.5f,  0.5f, textureCoords[5][3].x, textureCoords[5][3].y,
		 0.5f, -0.5f, -0.5f, textureCoords[5][2].x, textureCoords[5][2].y,
		 0.5f, -0.5f, -0.5f, textureCoords[5][2].x, textureCoords[5][2].y,
		 0.5f,  0.5f, -0.5f, textureCoords[5][1].x, textureCoords[5][1].y,
		 0.5f,  0.5f,  0.5f, textureCoords[5][0].x, textureCoords[5][0].y,

	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Block::render(const Shader &shader, const glm::mat4 &modelMatrix) const
{
	texture->bind();
	shader.use();
	glm::mat4 model = glm::translate(modelMatrix, position);
	shader.setMatrix4("model", model);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36); // Assuming VAO and VBO are already set up
	glBindVertexArray(0);

	texture->unbind();
}

bool Block::isUnder(const glm::vec3 &position) const
{
	return position.x >= this->position.x - 0.5f && position.x < this->position.x + 0.5f &&
		position.y >= this->position.y - 0.5f && position.y < this->position.y + 0.5f &&
		position.z >= this->position.z - 0.5f && position.z < this->position.z + 0.5f;
}

bool Block::checkFall(const glm::vec3 &position) const
{
	return position.y < this->position.y;
}