#include "Character.hpp"

Character::Character(const glm::vec3 &position, const glm::vec3 &up, float yaw, float pitch)
	: position(position), size(glm::vec3(0.6f, 1.8f, 0.6f)), camera(glm::vec3(position.x, position.y + 1.62f, position.z), up, yaw, pitch),
	  verticalVelocity(0.0f), isFlying(true), canGoUp(true), canGoDown(true),
	  walkSpeed(2.5f), flySpeed(5.0f), movementSpeed(flySpeed)
{
	texture = new Texture("../assets/textures/character.png");
	setTextureCoords();
	setupMesh();
}

void Character::setTextureCoords(){

	float unitWidth = 1.0f / texture->getWidth();
	float unitHeight = 1.0f / texture->getHeight();
	// The texture is 64x64 pixels
	// Head is formed by 6 8x8 squares
	// The body is formed by n 4x4 squares (the width is 8 pixels and the depth is 4 pixels)
	// Arms and legs are formed by n 4x4 squares (the width is 4 pixels and the depth is 4 pixels)
	// So let's divide the texture into 4x4 squares
	unsigned int subSquareWidth = 4;
	unsigned int subSquareHeight = 4;
	textureCoords = {
		// Head
		std::array<std::array<glm::vec2, 4>, 6> {
			// Front
			std::array<glm::vec2, 4>{
				glm::vec2(1 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-left
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-right
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight),                // Bottom-right
				glm::vec2(1 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight)                 // Bottom-left
			},
			// Back
			std::array<glm::vec2, 4>{
				glm::vec2(3 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-left
				glm::vec2(4 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-right
				glm::vec2(4 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight),                // Bottom-right
				glm::vec2(3 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight)                 // Bottom-left
			},
			// Left
			std::array<glm::vec2, 4>{
				glm::vec2(0 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-left
				glm::vec2(1 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-right
				glm::vec2(1 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight),                // Bottom-right
				glm::vec2(0 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight)                 // Bottom-left
			},
			// Right
			std::array<glm::vec2, 4>{
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-left
				glm::vec2(3 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Top-right
				glm::vec2(3 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight),                // Bottom-right
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 2 * subSquareHeight * 2 * unitHeight)                 // Bottom-left
			},
			// Top
			std::array<glm::vec2, 4>{
				glm::vec2(1 * subSquareWidth * 2 * unitWidth, 0 * subSquareHeight * 2 * unitHeight),                // Top-left
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 0 * subSquareHeight * 2 * unitHeight),                // Top-right
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Bottom-right
				glm::vec2(1 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight)                 // Bottom-left
			},
			// Bottom
			std::array<glm::vec2, 4>{
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 0 * subSquareHeight * 2 * unitHeight),                // Top-left
				glm::vec2(3 * subSquareWidth * 2 * unitWidth, 0 * subSquareHeight * 2 * unitHeight),                // Top-right
				glm::vec2(3 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight),                // Bottom-right
				glm::vec2(2 * subSquareWidth * 2 * unitWidth, 1 * subSquareHeight * 2 * unitHeight)                 // Bottom-left
			}
		},
		// Body
		std::array<std::array<glm::vec2, 4>, 6> {
			// Front
			std::array<glm::vec2, 4>{
				glm::vec2(5 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(7 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(7 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(5 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Back
			std::array<glm::vec2, 4>{
				glm::vec2(8  * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(10 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(10 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(8  * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Left
			std::array<glm::vec2, 4>{
				glm::vec2(4 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(6 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(6 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(4 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Right
			std::array<glm::vec2, 4>{
				glm::vec2(7 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(8 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(8 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(7 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Top
			std::array<glm::vec2, 4>{
				glm::vec2(5 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(7 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(7 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(5 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Bottom
			std::array<glm::vec2, 4>{
				glm::vec2(7 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(9 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(9 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(7 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight)                 // Bottom-left
			}
		},
		// Right Arm
		std::array<std::array<glm::vec2, 4>, 6> {
			// Front
			std::array<glm::vec2, 4>{
				glm::vec2(9  * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(10 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(10 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(9  * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Back
			std::array<glm::vec2, 4>{
				glm::vec2(11 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(12 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(12 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(11 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Left
			std::array<glm::vec2, 4>{
				glm::vec2(10 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(11 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(11 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(10 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Right
			std::array<glm::vec2, 4>{
				glm::vec2(8 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(9 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(9 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(8 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Top
			std::array<glm::vec2, 4>{
				glm::vec2(9  * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(10 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(10 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(9  * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Bottom
			std::array<glm::vec2, 4>{
				glm::vec2(10 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(11 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(11 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(10 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight)                 // Bottom-left
			}
		},
		// Left Arm
		std::array<std::array<glm::vec2, 4>, 6> {
			// Front
			std::array<glm::vec2, 4>{
				glm::vec2(11 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(12 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(12 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(11 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Back
			std::array<glm::vec2, 4>{
				glm::vec2(13 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(14 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(14 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(13 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Left
			std::array<glm::vec2, 4>{
				glm::vec2(12 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(13 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(13 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(12 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Right
			std::array<glm::vec2, 4>{
				glm::vec2(10 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(11 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(11 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(10 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Top
			std::array<glm::vec2, 4>{
				glm::vec2(11 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(12 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(12 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(11 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Bottom
			std::array<glm::vec2, 4>{
				glm::vec2(12 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(13 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(13 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(12 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight)                 // Bottom-left
			}
		},
		// Right Leg
		std::array<std::array<glm::vec2, 4>, 6> {
			// Front
			std::array<glm::vec2, 4>{
				glm::vec2(5 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(6 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(6 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(5 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Back
			std::array<glm::vec2, 4>{
				glm::vec2(7 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(8 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(8 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(7 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Left
			std::array<glm::vec2, 4>{
				glm::vec2(6 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(7 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(7 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(6 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Right
			std::array<glm::vec2, 4>{
				glm::vec2(4 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(5 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(5 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(4 * subSquareWidth * unitWidth, 16 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Top
			std::array<glm::vec2, 4>{
				glm::vec2(5 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(6 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(6 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(5 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Bottom
			std::array<glm::vec2, 4>{
				glm::vec2(6 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(7 * subSquareWidth * unitWidth, 12 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(7 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(6 * subSquareWidth * unitWidth, 13 * subSquareHeight * unitHeight)                 // Bottom-left
			}
		},
		// Left Leg
		std::array<std::array<glm::vec2, 4>, 6> {
			// Front
			std::array<glm::vec2, 4>{
				glm::vec2(1 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(2 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(2 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(1 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Back
			std::array<glm::vec2, 4>{
				glm::vec2(3 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(4 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(4 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(3 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Left
			std::array<glm::vec2, 4>{
				glm::vec2(2 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(3 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(3 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(2 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Right
			std::array<glm::vec2, 4>{
				glm::vec2(0 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(1 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(1 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(0 * subSquareWidth * unitWidth, 8 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Top
			std::array<glm::vec2, 4>{
				glm::vec2(1 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(2 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(2 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(1 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight)                 // Bottom-left
			},
			// Bottom
			std::array<glm::vec2, 4>{
				glm::vec2(2 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-left
				glm::vec2(3 * subSquareWidth * unitWidth, 4 * subSquareHeight * unitHeight),                // Top-right
				glm::vec2(3 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight),                // Bottom-right
				glm::vec2(2 * subSquareWidth * unitWidth, 5 * subSquareHeight * unitHeight)                 // Bottom-left
			}
		}
	};
}

void Character::setupMesh() {
	float vertices[] = {
		// Head (6 faces)
		// Front
		 0.3f, 2.4f, 0.3f,   textureCoords[0][0][0].x, textureCoords[0][0][0].y,
		-0.3f, 2.4f, 0.3f,   textureCoords[0][0][1].x, textureCoords[0][0][1].y,
		-0.3f, 1.8f, 0.3f,   textureCoords[0][0][2].x, textureCoords[0][0][2].y,
		-0.3f, 1.8f, 0.3f,   textureCoords[0][0][2].x, textureCoords[0][0][2].y,
		 0.3f, 1.8f, 0.3f,   textureCoords[0][0][3].x, textureCoords[0][0][3].y,
		 0.3f, 2.4f, 0.3f,   textureCoords[0][0][0].x, textureCoords[0][0][0].y,

		// Back
		-0.3f, 2.4f, -0.3f,  textureCoords[0][1][1].x, textureCoords[0][1][1].y,
		 0.3f, 2.4f, -0.3f,  textureCoords[0][1][0].x, textureCoords[0][1][0].y,
		 0.3f, 1.8f, -0.3f,  textureCoords[0][1][3].x, textureCoords[0][1][3].y,
		 0.3f, 1.8f, -0.3f,  textureCoords[0][1][3].x, textureCoords[0][1][3].y,
		-0.3f, 1.8f, -0.3f,  textureCoords[0][1][2].x, textureCoords[0][1][2].y,
		-0.3f, 2.4f, -0.3f,  textureCoords[0][1][1].x, textureCoords[0][1][1].y,

		// Left
		-0.3f, 2.4f, 0.3f,   textureCoords[0][2][1].x, textureCoords[0][2][1].y,
		-0.3f, 2.4f, -0.3f,  textureCoords[0][2][0].x, textureCoords[0][2][0].y,
		-0.3f, 1.8f, -0.3f,  textureCoords[0][2][3].x, textureCoords[0][2][3].y,
		-0.3f, 1.8f, -0.3f,  textureCoords[0][2][3].x, textureCoords[0][2][3].y,
		-0.3f, 1.8f, 0.3f,   textureCoords[0][2][2].x, textureCoords[0][2][2].y,
		-0.3f, 2.4f, 0.3f,   textureCoords[0][2][1].x, textureCoords[0][2][1].y,

		// Right
		 0.3f, 2.4f, -0.3f,  textureCoords[0][3][1].x, textureCoords[0][3][1].y,
		 0.3f, 2.4f, 0.3f,   textureCoords[0][3][0].x, textureCoords[0][3][0].y,
		 0.3f, 1.8f, 0.3f,   textureCoords[0][3][3].x, textureCoords[0][3][3].y,
		 0.3f, 1.8f, 0.3f,   textureCoords[0][3][3].x, textureCoords[0][3][3].y,
		 0.3f, 1.8f, -0.3f,  textureCoords[0][3][2].x, textureCoords[0][3][2].y,
		 0.3f, 2.4f, -0.3f,  textureCoords[0][3][1].x, textureCoords[0][3][1].y,

		// Top
		 0.3f, 2.4f, -0.3f,  textureCoords[0][4][1].x, textureCoords[0][4][1].y,
		-0.3f, 2.4f, -0.3f,  textureCoords[0][4][0].x, textureCoords[0][4][0].y,
		-0.3f, 2.4f, 0.3f,   textureCoords[0][4][3].x, textureCoords[0][4][3].y,
		-0.3f, 2.4f, 0.3f,   textureCoords[0][4][3].x, textureCoords[0][4][3].y,
		 0.3f, 2.4f, 0.3f,   textureCoords[0][4][2].x, textureCoords[0][4][2].y,
		 0.3f, 2.4f, -0.3f,  textureCoords[0][4][1].x, textureCoords[0][4][1].y,


		// Bottom
		-0.3f, 1.8f, -0.3f,  textureCoords[0][5][0].x, textureCoords[0][5][0].y,
		 0.3f, 1.8f, -0.3f,  textureCoords[0][5][1].x, textureCoords[0][5][1].y,
		 0.3f, 1.8f, 0.3f,   textureCoords[0][5][2].x, textureCoords[0][5][2].y,
		 0.3f, 1.8f, 0.3f,   textureCoords[0][5][2].x, textureCoords[0][5][2].y,
		-0.3f, 1.8f, 0.3f,   textureCoords[0][5][3].x, textureCoords[0][5][3].y,
		-0.3f, 1.8f, -0.3f,  textureCoords[0][5][0].x, textureCoords[0][5][0].y,

		// Body (6 faces)
		// Front
		-0.3f, 0.9f, 0.15f,  textureCoords[1][0][3].x, textureCoords[1][0][3].y,
		 0.3f, 0.9f, 0.15f,  textureCoords[1][0][2].x, textureCoords[1][0][2].y,
		 0.3f, 1.8f, 0.15f,  textureCoords[1][0][1].x, textureCoords[1][0][1].y,
		 0.3f, 1.8f, 0.15f,  textureCoords[1][0][1].x, textureCoords[1][0][1].y,
		-0.3f, 1.8f, 0.15f,  textureCoords[1][0][0].x, textureCoords[1][0][0].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[1][0][3].x, textureCoords[1][0][3].y,

		// Back
		-0.3f, 1.8f, -0.15f, textureCoords[1][1][1].x, textureCoords[1][1][1].y,
		 0.3f, 1.8f, -0.15f, textureCoords[1][1][0].x, textureCoords[1][1][0].y,
		 0.3f, 0.9f, -0.15f, textureCoords[1][1][3].x, textureCoords[1][1][3].y,
		 0.3f, 0.9f, -0.15f, textureCoords[1][1][3].x, textureCoords[1][1][3].y,
		-0.3f, 0.9f, -0.15f, textureCoords[1][1][2].x, textureCoords[1][1][2].y,
		-0.3f, 1.8f, -0.15f, textureCoords[1][1][1].x, textureCoords[1][1][1].y,

		// Left
		-0.3f, 1.8f, -0.15f, textureCoords[1][2][0].x, textureCoords[1][2][0].y,
		-0.3f, 1.8f, 0.15f,  textureCoords[1][2][1].x, textureCoords[1][2][1].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[1][2][2].x, textureCoords[1][2][2].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[1][2][2].x, textureCoords[1][2][2].y,
		-0.3f, 0.9f, -0.15f, textureCoords[1][2][3].x, textureCoords[1][2][3].y,
		-0.3f, 1.8f, -0.15f, textureCoords[1][2][0].x, textureCoords[1][2][0].y,

		// Right
		 0.3f, 1.8f, -0.15f, textureCoords[1][3][0].x, textureCoords[1][3][0].y,
		 0.3f, 1.8f, 0.15f,  textureCoords[1][3][1].x, textureCoords[1][3][1].y,
		 0.3f, 0.9f, 0.15f,  textureCoords[1][3][2].x, textureCoords[1][3][2].y,
		 0.3f, 0.9f, 0.15f,  textureCoords[1][3][2].x, textureCoords[1][3][2].y,
		 0.3f, 0.9f, -0.15f, textureCoords[1][3][3].x, textureCoords[1][3][3].y,
		 0.3f, 1.8f, -0.15f, textureCoords[1][3][0].x, textureCoords[1][3][0].y,

		// Top
		-0.3f, 1.8f, -0.15f, textureCoords[1][4][0].x, textureCoords[1][4][0].y,
		 0.3f, 1.8f, -0.15f, textureCoords[1][4][1].x, textureCoords[1][4][1].y,
		 0.3f, 1.8f, 0.15f,  textureCoords[1][4][2].x, textureCoords[1][4][2].y,
		 0.3f, 1.8f, 0.15f,  textureCoords[1][4][2].x, textureCoords[1][4][2].y,
		-0.3f, 1.8f, 0.15f,  textureCoords[1][4][3].x, textureCoords[1][4][3].y,
		-0.3f, 1.8f, -0.15f, textureCoords[1][4][0].x, textureCoords[1][4][0].y,

		// Bottom
		-0.3f, 0.9f, -0.15f, textureCoords[1][5][0].x, textureCoords[1][5][0].y,
		 0.3f, 0.9f, -0.15f, textureCoords[1][5][1].x, textureCoords[1][5][1].y,
		 0.3f, 0.9f, 0.15f,  textureCoords[1][5][2].x, textureCoords[1][5][2].y,
		 0.3f, 0.9f, 0.15f,  textureCoords[1][5][2].x, textureCoords[1][5][2].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[1][5][3].x, textureCoords[1][5][3].y,
		-0.3f, 0.9f, -0.15f, textureCoords[1][5][0].x, textureCoords[1][5][0].y,

		// Left Arm (6 faces)
		// Front
		-0.3f, 1.8f, 0.15f,  textureCoords[2][0][0].x, textureCoords[2][0][0].y,
		-0.6f, 1.8f, 0.15f, textureCoords[2][0][1].x, textureCoords[2][0][1].y,
		-0.6f, 0.9f, 0.15f, textureCoords[2][0][2].x, textureCoords[2][0][2].y,
		-0.6f, 0.9f, 0.15f, textureCoords[2][0][2].x, textureCoords[2][0][2].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[2][0][3].x, textureCoords[2][0][3].y,
		-0.3f, 1.8f, 0.15f,  textureCoords[2][0][0].x, textureCoords[2][0][0].y,

		// Back
		-0.6f, 1.8f, -0.15f, textureCoords[2][1][0].x, textureCoords[2][1][0].y,
		-0.3f, 1.8f, -0.15f,  textureCoords[2][1][1].x, textureCoords[2][1][1].y,
		-0.3f, 0.9f, -0.15f,  textureCoords[2][1][2].x, textureCoords[2][1][2].y,
		-0.3f, 0.9f, -0.15f,  textureCoords[2][1][2].x, textureCoords[2][1][2].y,
		-0.6f, 0.9f, -0.15f, textureCoords[2][1][3].x, textureCoords[2][1][3].y,
		-0.6f, 1.8f, -0.15f, textureCoords[2][1][0].x, textureCoords[2][1][0].y,

		// Left
		-0.6f, 1.8f, 0.15f,  textureCoords[2][2][0].x, textureCoords[2][2][0].y,
		-0.6f, 1.8f, -0.15f, textureCoords[2][2][1].x, textureCoords[2][2][1].y,
		-0.6f, 0.9f, -0.15f, textureCoords[2][2][2].x, textureCoords[2][2][2].y,
		-0.6f, 0.9f, -0.15f, textureCoords[2][2][2].x, textureCoords[2][2][2].y,
		-0.6f, 0.9f, 0.15f,  textureCoords[2][2][3].x, textureCoords[2][2][3].y,
		-0.6f, 1.8f, 0.15f,  textureCoords[2][2][0].x, textureCoords[2][2][0].y,

		// Right
		-0.3f, 1.8f, -0.15f, textureCoords[2][3][0].x, textureCoords[2][3][0].y,
		-0.3f, 1.8f, 0.15f,  textureCoords[2][3][1].x, textureCoords[2][3][1].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[2][3][2].x, textureCoords[2][3][2].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[2][3][2].x, textureCoords[2][3][2].y,
		-0.3f, 0.9f, -0.15f, textureCoords[2][3][3].x, textureCoords[2][3][3].y,
		-0.3f, 1.8f, -0.15f, textureCoords[2][3][0].x, textureCoords[2][3][0].y,

		// Top
		-0.3f, 1.8f, -0.15f, textureCoords[2][4][0].x, textureCoords[2][4][0].y,
		-0.6f, 1.8f, -0.15f,textureCoords[2][4][1].x, textureCoords[2][4][1].y,
		-0.6f, 1.8f, 0.15f, textureCoords[2][4][2].x, textureCoords[2][4][2].y,
		-0.6f, 1.8f, 0.15f, textureCoords[2][4][2].x, textureCoords[2][4][2].y,
		-0.3f, 1.8f, 0.15f,  textureCoords[2][4][3].x, textureCoords[2][4][3].y,
		-0.3f, 1.8f, -0.15f, textureCoords[2][4][0].x, textureCoords[2][4][0].y,


		// Bottom
		-0.6f, 0.9f, -0.15f, textureCoords[2][5][0].x, textureCoords[2][5][0].y,
		-0.3f, 0.9f, -0.15f,  textureCoords[2][5][1].x, textureCoords[2][5][1].y,
		-0.3f, 0.9f, 0.15f,   textureCoords[2][5][2].x, textureCoords[2][5][2].y,
		-0.3f, 0.9f, 0.15f,   textureCoords[2][5][2].x, textureCoords[2][5][2].y,
		-0.6f, 0.9f, 0.15f,  textureCoords[2][5][3].x, textureCoords[2][5][3].y,
		-0.6f, 0.9f, -0.15f, textureCoords[2][5][0].x, textureCoords[2][5][0].y,

		// Right Arm (6 faces)
		// Front
		0.6f, 1.8f, 0.15f,  textureCoords[3][0][0].x, textureCoords[3][0][0].y,
		0.3f, 1.8f, 0.15f,   textureCoords[3][0][1].x, textureCoords[3][0][1].y,
		0.3f, 0.9f, 0.15f,   textureCoords[3][0][2].x, textureCoords[3][0][2].y,
		0.3f, 0.9f, 0.15f,   textureCoords[3][0][2].x, textureCoords[3][0][2].y,
		0.6f, 0.9f, 0.15f,  textureCoords[3][0][3].x, textureCoords[3][0][3].y,
		0.6f, 1.8f, 0.15f,  textureCoords[3][0][0].x, textureCoords[3][0][0].y,


		// Back
		0.3f, 1.8f, -0.15f, textureCoords[3][1][0].x, textureCoords[3][1][0].y,
		0.6f, 1.8f, -0.15f, textureCoords[3][1][1].x, textureCoords[3][1][1].y,
		0.6f, 0.9f, -0.15f, textureCoords[3][1][2].x, textureCoords[3][1][2].y,
		0.6f, 0.9f, -0.15f, textureCoords[3][1][2].x, textureCoords[3][1][2].y,
		0.3f, 0.9f, -0.15f,  textureCoords[3][1][3].x, textureCoords[3][1][3].y,
		0.3f, 1.8f, -0.15f,  textureCoords[3][1][0].x, textureCoords[3][1][0].y,

		// Left
		0.3f, 1.8f, -0.15f, textureCoords[3][2][0].x, textureCoords[3][2][0].y,
		0.3f, 1.8f, 0.15f,  textureCoords[3][2][1].x, textureCoords[3][2][1].y,
		0.3f, 0.9f, 0.15f,  textureCoords[3][2][2].x, textureCoords[3][2][2].y,
		0.3f, 0.9f, 0.15f,  textureCoords[3][2][2].x, textureCoords[3][2][2].y,
		0.3f, 0.9f, -0.15f, textureCoords[3][2][3].x, textureCoords[3][2][3].y,
		0.3f, 1.8f, -0.15f, textureCoords[3][2][0].x, textureCoords[3][2][0].y,

		// Right
		0.6f, 1.8f, -0.15f, textureCoords[3][3][0].x, textureCoords[3][3][0].y,
		0.6f, 1.8f, 0.15f,  textureCoords[3][3][1].x, textureCoords[3][3][1].y,
		0.6f, 0.9f, 0.15f,  textureCoords[3][3][2].x, textureCoords[3][3][2].y,
		0.6f, 0.9f, 0.15f,  textureCoords[3][3][2].x, textureCoords[3][3][2].y,
		0.6f, 0.9f, -0.15f, textureCoords[3][3][3].x, textureCoords[3][3][3].y,
		0.6f, 1.8f, -0.15f, textureCoords[3][3][0].x, textureCoords[3][3][0].y,

		// Top
		0.6f, 1.8f, -0.15f, textureCoords[3][4][0].x, textureCoords[3][4][0].y,
		0.3f, 1.8f, -0.15f, textureCoords[3][4][1].x, textureCoords[3][4][1].y,
		0.3f, 1.8f, 0.15f,  textureCoords[3][4][2].x, textureCoords[3][4][2].y,
		0.3f, 1.8f, 0.15f,  textureCoords[3][4][2].x, textureCoords[3][4][2].y,
		0.6f, 1.8f, 0.15f, textureCoords[3][4][3].x, textureCoords[3][4][3].y,
		0.6f, 1.8f, -0.15f, textureCoords[3][4][0].x, textureCoords[3][4][0].y,

		// Bottom
		0.3f, 0.9f, -0.15f, textureCoords[3][5][0].x, textureCoords[3][5][0].y,
		0.6f, 0.9f, -0.15f, textureCoords[3][5][1].x, textureCoords[3][5][1].y,
		0.6f, 0.9f, 0.15f,  textureCoords[3][5][2].x, textureCoords[3][5][2].y,
		0.6f, 0.9f, 0.15f,  textureCoords[3][5][2].x, textureCoords[3][5][2].y,
		0.3f, 0.9f, 0.15f,   textureCoords[3][5][3].x, textureCoords[3][5][3].y,
		0.3f, 0.9f, -0.15f,  textureCoords[3][5][0].x, textureCoords[3][5][0].y,

		// Left Leg (6 faces)
		// Front
		0.0f, 0.9f, 0.15f,  textureCoords[4][0][0].x, textureCoords[4][0][0].y,
		-0.3f, 0.9f, 0.15f,  textureCoords[4][0][1].x, textureCoords[4][0][1].y,
		-0.3f, 0.0f, 0.15f,  textureCoords[4][0][2].x, textureCoords[4][0][2].y,
		-0.3f, 0.0f, 0.15f,  textureCoords[4][0][2].x, textureCoords[4][0][2].y,
		0.0f, 0.0f, 0.15f,  textureCoords[4][0][3].x, textureCoords[4][0][3].y,
		0.0f, 0.9f, 0.15f,  textureCoords[4][0][0].x, textureCoords[4][0][0].y,

		// Back
		-0.3f, 0.9f, -0.15f, textureCoords[4][1][0].x, textureCoords[4][1][0].y,
		 0.0f, 0.9f, -0.15f, textureCoords[4][1][1].x, textureCoords[4][1][1].y,
		 0.0f, 0.0f, -0.15f, textureCoords[4][1][2].x, textureCoords[4][1][2].y,
		 0.0f, 0.0f, -0.15f, textureCoords[4][1][2].x, textureCoords[4][1][2].y,
		-0.3f, 0.0f, -0.15f, textureCoords[4][1][3].x, textureCoords[4][1][3].y,
		-0.3f, 0.9f, -0.15f, textureCoords[4][1][0].x, textureCoords[4][1][0].y,

		// Left
		-0.3f, 0.9f, 0.15f, textureCoords[4][2][0].x, textureCoords[4][2][0].y,
		-0.3f, 0.9f, -0.15f, textureCoords[4][2][1].x, textureCoords[4][2][1].y,
		-0.3f, 0.0f, -0.15f, textureCoords[4][2][2].x, textureCoords[4][2][2].y,
		-0.3f, 0.0f, -0.15f, textureCoords[4][2][2].x, textureCoords[4][2][2].y,
		-0.3f, 0.0f, 0.15f, textureCoords[4][2][3].x, textureCoords[4][2][3].y,
		-0.3f, 0.9f, 0.15f, textureCoords[4][2][0].x, textureCoords[4][2][0].y,

		// Right
		0.0f, 0.9f, -0.15f, textureCoords[4][3][0].x, textureCoords[4][3][0].y,
		0.0f, 0.9f, 0.15f,  textureCoords[4][3][1].x, textureCoords[4][3][1].y,
		0.0f, 0.0f, 0.15f,  textureCoords[4][3][2].x, textureCoords[4][3][2].y,
		0.0f, 0.0f, 0.15f,  textureCoords[4][3][2].x, textureCoords[4][3][2].y,
		0.0f, 0.0f, -0.15f, textureCoords[4][3][3].x, textureCoords[4][3][3].y,
		0.0f, 0.9f, -0.15f, textureCoords[4][3][0].x, textureCoords[4][3][0].y,

		// Top
		0.0f, 0.9f, -0.15f, textureCoords[4][4][0].x, textureCoords[4][4][0].y,
		-0.3f, 0.9f, -0.15f, textureCoords[4][4][1].x, textureCoords[4][4][1].y,
		-0.3f, 0.9f, 0.15f, textureCoords[4][4][2].x, textureCoords[4][4][2].y,
		-0.3f, 0.9f, 0.15f, textureCoords[4][4][2].x, textureCoords[4][4][2].y,
		0.0f, 0.9f, 0.15f, textureCoords[4][4][3].x, textureCoords[4][4][3].y,
		0.0f, 0.9f, -0.15f, textureCoords[4][4][0].x, textureCoords[4][4][0].y,

		// Bottom
		-0.3f, 0.0f, -0.15f, textureCoords[4][5][0].x, textureCoords[4][5][0].y,
		 0.0f, 0.0f, -0.15f, textureCoords[4][5][1].x, textureCoords[4][5][1].y,
		 0.0f, 0.0f, 0.15f,  textureCoords[4][5][2].x, textureCoords[4][5][2].y,
		 0.0f, 0.0f, 0.15f,  textureCoords[4][5][2].x, textureCoords[4][5][2].y,
		-0.3f, 0.0f, 0.15f,  textureCoords[4][5][3].x, textureCoords[4][5][3].y,
		-0.3f, 0.0f, -0.15f, textureCoords[4][5][0].x, textureCoords[4][5][0].y,

		// Right Leg (6 faces)
		// Front
		0.3f, 0.9f, 0.15f, textureCoords[5][0][0].x, textureCoords[5][0][0].y,
		0.0f, 0.9f, 0.15f, textureCoords[5][0][1].x, textureCoords[5][0][1].y,
		0.0f, 0.0f, 0.15f, textureCoords[5][0][2].x, textureCoords[5][0][2].y,
		0.0f, 0.0f, 0.15f, textureCoords[5][0][2].x, textureCoords[5][0][2].y,
		0.3f, 0.0f, 0.15f, textureCoords[5][0][3].x, textureCoords[5][0][3].y,
		0.3f, 0.9f, 0.15f, textureCoords[5][0][0].x, textureCoords[5][0][0].y,

		// Back
		0.0f, 0.9f, -0.15f, textureCoords[5][1][0].x, textureCoords[5][1][0].y,
		0.3f, 0.9f, -0.15f, textureCoords[5][1][1].x, textureCoords[5][1][1].y,
		0.3f, 0.0f, -0.15f, textureCoords[5][1][2].x, textureCoords[5][1][2].y,
		0.3f, 0.0f, -0.15f, textureCoords[5][1][2].x, textureCoords[5][1][2].y,
		0.0f, 0.0f, -0.15f, textureCoords[5][1][3].x, textureCoords[5][1][3].y,
		0.0f, 0.9f, -0.15f, textureCoords[5][1][0].x, textureCoords[5][1][0].y,

		// Left
		0.0f, 0.9f, -0.15f, textureCoords[5][2][0].x, textureCoords[5][2][0].y,
		0.0f, 0.9f, 0.15f,  textureCoords[5][2][1].x, textureCoords[5][2][1].y,
		0.0f, 0.0f, 0.15f,  textureCoords[5][2][2].x, textureCoords[5][2][2].y,
		0.0f, 0.0f, 0.15f,  textureCoords[5][2][2].x, textureCoords[5][2][2].y,
		0.0f, 0.0f, -0.15f, textureCoords[5][2][3].x, textureCoords[5][2][3].y,
		0.0f, 0.9f, -0.15f, textureCoords[5][2][0].x, textureCoords[5][2][0].y,

		// Right
		0.3f, 0.9f, -0.15f, textureCoords[5][3][0].x, textureCoords[5][3][0].y,
		0.3f, 0.9f, 0.15f,  textureCoords[5][3][1].x, textureCoords[5][3][1].y,
		0.3f, 0.0f, 0.15f,  textureCoords[5][3][2].x, textureCoords[5][3][2].y,
		0.3f, 0.0f, 0.15f,  textureCoords[5][3][2].x, textureCoords[5][3][2].y,
		0.3f, 0.0f, -0.15f, textureCoords[5][3][3].x, textureCoords[5][3][3].y,
		0.3f, 0.9f, -0.15f, textureCoords[5][3][0].x, textureCoords[5][3][0].y,

		// Top
		0.3f, 0.9f, -0.15f, textureCoords[5][4][0].x, textureCoords[5][4][0].y,
		0.0f, 0.9f, -0.15f, textureCoords[5][4][1].x, textureCoords[5][4][1].y,
		0.0f, 0.9f, 0.15f, textureCoords[5][4][2].x, textureCoords[5][4][2].y,
		0.0f, 0.9f, 0.15f, textureCoords[5][4][2].x, textureCoords[5][4][2].y,
		0.3f, 0.9f, 0.15f, textureCoords[5][4][3].x, textureCoords[5][4][3].y,
		0.3f, 0.9f, -0.15f, textureCoords[5][4][0].x, textureCoords[5][4][0].y,

		// Bottom
		0.0f, 0.0f, -0.15f, textureCoords[5][5][0].x, textureCoords[5][5][0].y,
		0.3f, 0.0f, -0.15f, textureCoords[5][5][1].x, textureCoords[5][5][1].y,
		0.3f, 0.0f, 0.15f,  textureCoords[5][5][2].x, textureCoords[5][5][2].y,
		0.3f, 0.0f, 0.15f,  textureCoords[5][5][2].x, textureCoords[5][5][2].y,
		0.0f, 0.0f, 0.15f,  textureCoords[5][5][3].x, textureCoords[5][5][3].y,
		0.0f, 0.0f, -0.15f, textureCoords[5][5][0].x, textureCoords[5][5][0].y,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}



void Character::processKeyboard(int direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	glm::vec3 horizontalFront = glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z));

	if (direction == 0) // forward
		position += horizontalFront * velocity;
	if (direction == 1) // backward
		position -= horizontalFront * velocity;
	if (direction == 2) // left
		position -= camera.getRight() * velocity;
	if (direction == 3) // right
		position += camera.getRight() * velocity;
	if (isFlying)
	{
		if (direction == 4 && canGoUp) // up
			position += camera.getWorldUp() * velocity;
		if (direction == 5 && canGoDown) // down
			position -= camera.getWorldUp() * velocity;
	}
	camera.setPosition(glm::vec3(position.x, position.y + 1.62f, position.z));
}

void Character::processMouseMovement(float xoffset, float yoffset)
{
	camera.processMouseMovement(xoffset, yoffset);
}

void Character::update(World &world, float deltaTime)
{
	Physics::applyGravity(world, position, verticalVelocity, deltaTime, isFlying, isJumping, canGoUp, canGoDown);
	camera.setPosition(glm::vec3(position.x, position.y + 1.8f, position.z));
	movementSpeed = isFlying ? flySpeed : walkSpeed;
}

void Character::render(const Shader &shader)
{
	texture->bind();
	shader.use();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
	model = glm::translate(model, glm::vec3(0.0f));
	shader.setMatrix4("model", model);

	glBindVertexArray(VAO);
	// Chaque partie du personnage a 36 vertices et il y a 6 parties (tête, corps, bras gauche, bras droit, jambe gauche, jambe droite)
	glDrawArrays(GL_TRIANGLES, 0, 36 * 6);
	glBindVertexArray(0);
}


void Character::jump()
{
	if (!isFlying && verticalVelocity == 0.0f)
	{
		verticalVelocity = 9.0f;
		isJumping = true;
	}
}

void Character::setFlying(bool flying)
{
	isFlying = flying;
}

bool Character::getFlying() const
{
	return isFlying;
}

glm::vec3 Character::getPosition() const
{
	return position;
}

glm::mat4 Character::getViewMatrix() const
{
	return camera.getViewMatrix();
}
