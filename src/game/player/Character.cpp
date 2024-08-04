#include "Character.hpp"
#include "../physics/Physics.hpp"
#include <GL/glew.h>

Character::Character(const glm::vec3 &position, const glm::vec3 &up, float yaw, float pitch)
	: position(position), size(glm::vec3(0.6f, 1.8f, 0.6f)), camera(glm::vec3(position.x, position.y + 1.62f, position.z), up, yaw, pitch),
	  verticalVelocity(0.0f), isFlying(true), canGoUp(true), canGoDown(true),
	  walkSpeed(2.5f), flySpeed(5.0f), movementSpeed(flySpeed)
{
	setupMesh();
}

void Character::setupMesh() {
	// Define the vertices for a simple character model
	float vertices[] = {
		// Head (6 faces)
		// Front
		-0.3f, 2.4f, 0.3f,   0.3f, 2.4f, 0.3f,   0.3f, 1.8f, 0.3f,
		 0.3f, 1.8f, 0.3f,  -0.3f, 1.8f, 0.3f,  -0.3f, 2.4f, 0.3f,

		// Back
		-0.3f, 2.4f, -0.3f,  0.3f, 2.4f, -0.3f,  0.3f, 1.8f, -0.3f,
		 0.3f, 1.8f, -0.3f, -0.3f, 1.8f, -0.3f, -0.3f, 2.4f, -0.3f,

		// Left
		-0.3f, 2.4f, -0.3f, -0.3f, 2.4f, 0.3f,  -0.3f, 1.8f, 0.3f,
		-0.3f, 1.8f, 0.3f,  -0.3f, 1.8f, -0.3f, -0.3f, 2.4f, -0.3f,

		// Right
		 0.3f, 2.4f, -0.3f,  0.3f, 2.4f, 0.3f,   0.3f, 1.8f, 0.3f,
		 0.3f, 1.8f, 0.3f,   0.3f, 1.8f, -0.3f,  0.3f, 2.4f, -0.3f,

		// Top
		-0.3f, 2.4f, -0.3f,  0.3f, 2.4f, -0.3f,  0.3f, 2.4f, 0.3f,
		 0.3f, 2.4f, 0.3f,  -0.3f, 2.4f, 0.3f,  -0.3f, 2.4f, -0.3f,

		// Bottom
		-0.3f, 1.8f, -0.3f,  0.3f, 1.8f, -0.3f,  0.3f, 1.8f, 0.3f,
		 0.3f, 1.8f, 0.3f,  -0.3f, 1.8f, 0.3f,  -0.3f, 1.8f, -0.3f,

		// Body (6 faces)
		// Front
		-0.3f, 1.8f, 0.15f,  0.3f, 1.8f, 0.15f,  0.3f, 0.6f, 0.15f,
		 0.3f, 0.6f, 0.15f, -0.3f, 0.6f, 0.15f, -0.3f, 1.8f, 0.15f,

		// Back
		-0.3f, 1.8f, -0.15f, 0.3f, 1.8f, -0.15f, 0.3f, 0.6f, -0.15f,
		 0.3f, 0.6f, -0.15f, -0.3f, 0.6f, -0.15f, -0.3f, 1.8f, -0.15f,

		// Left
		-0.3f, 1.8f, -0.15f, -0.3f, 1.8f, 0.15f, -0.3f, 0.6f, 0.15f,
		-0.3f, 0.6f, 0.15f, -0.3f, 0.6f, -0.15f, -0.3f, 1.8f, -0.15f,

		// Right
		 0.3f, 1.8f, -0.15f, 0.3f, 1.8f, 0.15f,  0.3f, 0.6f, 0.15f,
		 0.3f, 0.6f, 0.15f,  0.3f, 0.6f, -0.15f, 0.3f, 1.8f, -0.15f,

		// Top
		-0.3f, 1.8f, -0.15f, 0.3f, 1.8f, -0.15f, 0.3f, 1.8f, 0.15f,
		 0.3f, 1.8f, 0.15f, -0.3f, 1.8f, 0.15f, -0.3f, 1.8f, -0.15f,

		// Bottom
		-0.3f, 0.6f, -0.15f, 0.3f, 0.6f, -0.15f, 0.3f, 0.6f, 0.15f,
		 0.3f, 0.6f, 0.15f, -0.3f, 0.6f, 0.15f, -0.3f, 0.6f, -0.15f,

		// Left Arm (6 faces)
		// Front
		-0.45f, 1.8f, 0.15f, -0.3f, 1.8f, 0.15f, -0.3f, 0.9f, 0.15f,
		-0.3f, 0.9f, 0.15f, -0.45f, 0.9f, 0.15f, -0.45f, 1.8f, 0.15f,

		// Back
		-0.45f, 1.8f, -0.15f, -0.3f, 1.8f, -0.15f, -0.3f, 0.9f, -0.15f,
		-0.3f, 0.9f, -0.15f, -0.45f, 0.9f, -0.15f, -0.45f, 1.8f, -0.15f,

		// Left
		-0.45f, 1.8f, -0.15f, -0.45f, 1.8f, 0.15f, -0.45f, 0.9f, 0.15f,
		-0.45f, 0.9f, 0.15f, -0.45f, 0.9f, -0.15f, -0.45f, 1.8f, -0.15f,

		// Right
		-0.3f, 1.8f, -0.15f, -0.3f, 1.8f, 0.15f, -0.3f, 0.9f, 0.15f,
		-0.3f, 0.9f, 0.15f, -0.3f, 0.9f, -0.15f, -0.3f, 1.8f, -0.15f,

		// Top
		-0.45f, 1.8f, -0.15f, -0.3f, 1.8f, -0.15f, -0.3f, 1.8f, 0.15f,
		-0.3f, 1.8f, 0.15f, -0.45f, 1.8f, 0.15f, -0.45f, 1.8f, -0.15f,

		// Bottom
		-0.45f, 0.9f, -0.15f, -0.3f, 0.9f, -0.15f, -0.3f, 0.9f, 0.15f,
		-0.3f, 0.9f, 0.15f, -0.45f, 0.9f, 0.15f, -0.45f, 0.9f, -0.15f,

		// Right Arm (6 faces)
		// Front
		0.3f, 1.8f, 0.15f, 0.45f, 1.8f, 0.15f, 0.45f, 0.9f, 0.15f,
		0.45f, 0.9f, 0.15f, 0.3f, 0.9f, 0.15f, 0.3f, 1.8f, 0.15f,

		// Back
		0.3f, 1.8f, -0.15f, 0.45f, 1.8f, -0.15f, 0.45f, 0.9f, -0.15f,
		0.45f, 0.9f, -0.15f, 0.3f, 0.9f, -0.15f, 0.3f, 1.8f, -0.15f,

		// Left
		0.3f, 1.8f, -0.15f, 0.3f, 1.8f, 0.15f, 0.3f, 0.9f, 0.15f,
		0.3f, 0.9f, 0.15f, 0.3f, 0.9f, -0.15f, 0.3f, 1.8f, -0.15f,

		// Right
		0.45f, 1.8f, -0.15f, 0.45f, 1.8f, 0.15f, 0.45f, 0.9f, 0.15f,
		0.45f, 0.9f, 0.15f, 0.45f, 0.9f, -0.15f, 0.45f, 1.8f, -0.15f,

		// Top
		0.3f, 1.8f, -0.15f, 0.45f, 1.8f, -0.15f, 0.45f, 1.8f, 0.15f,
		0.45f, 1.8f, 0.15f, 0.3f, 1.8f, 0.15f, 0.3f, 1.8f, -0.15f,

		// Bottom
		0.3f, 0.9f, -0.15f, 0.45f, 0.9f, -0.15f, 0.45f, 0.9f, 0.15f,
		0.45f, 0.9f, 0.15f, 0.3f, 0.9f, 0.15f, 0.3f, 0.9f, -0.15f,

		// Left Leg (6 faces)
		// Front
		-0.15f, 0.9f, 0.15f,  0.15f, 0.9f, 0.15f,  0.15f, 0.0f, 0.15f,
		 0.15f, 0.0f, 0.15f, -0.15f, 0.0f, 0.15f, -0.15f, 0.9f, 0.15f,

		// Back
		-0.15f, 0.9f, -0.15f, 0.15f, 0.9f, -0.15f, 0.15f, 0.0f, -0.15f,
		 0.15f, 0.0f, -0.15f, -0.15f, 0.0f, -0.15f, -0.15f, 0.9f, -0.15f,

		// Left
		-0.15f, 0.9f, -0.15f, -0.15f, 0.9f, 0.15f, -0.15f, 0.0f, 0.15f,
		-0.15f, 0.0f, 0.15f, -0.15f, 0.0f, -0.15f, -0.15f, 0.9f, -0.15f,

		// Right
		0.15f, 0.9f, -0.15f, 0.15f, 0.9f, 0.15f, 0.15f, 0.0f, 0.15f,
		0.15f, 0.0f, 0.15f, 0.15f, 0.0f, -0.15f, 0.15f, 0.9f, -0.15f,

		// Top
		-0.15f, 0.9f, -0.15f, 0.15f, 0.9f, -0.15f, 0.15f, 0.9f, 0.15f,
		0.15f, 0.9f, 0.15f, -0.15f, 0.9f, 0.15f, -0.15f, 0.9f, -0.15f,

		// Bottom
		-0.15f, 0.0f, -0.15f, 0.15f, 0.0f, -0.15f, 0.15f, 0.0f, 0.15f,
		0.15f, 0.0f, 0.15f, -0.15f, 0.0f, 0.15f, -0.15f, 0.0f, -0.15f,

		// Right Leg (6 faces)
		// Front
		0.15f, 0.9f, 0.15f,  0.45f, 0.9f, 0.15f,  0.45f, 0.0f, 0.15f,
		0.45f, 0.0f, 0.15f,  0.15f, 0.0f, 0.15f,  0.15f, 0.9f, 0.15f,

		// Back
		0.15f, 0.9f, -0.15f, 0.45f, 0.9f, -0.15f, 0.45f, 0.0f, -0.15f,
		0.45f, 0.0f, -0.15f, 0.15f, 0.0f, -0.15f, 0.15f, 0.9f, -0.15f,

		// Left
		0.15f, 0.9f, -0.15f, 0.15f, 0.9f, 0.15f, 0.15f, 0.0f, 0.15f,
		0.15f, 0.0f, 0.15f, 0.15f, 0.0f, -0.15f, 0.15f, 0.9f, -0.15f,

		// Right
		0.45f, 0.9f, -0.15f, 0.45f, 0.9f, 0.15f, 0.45f, 0.0f, 0.15f,
		0.45f, 0.0f, 0.15f, 0.45f, 0.0f, -0.15f, 0.45f, 0.9f, -0.15f,

		// Top
		0.15f, 0.9f, -0.15f, 0.45f, 0.9f, -0.15f, 0.45f, 0.9f, 0.15f,
		0.45f, 0.9f, 0.15f, 0.15f, 0.9f, 0.15f, 0.15f, 0.9f, -0.15f,

		// Bottom
		0.15f, 0.0f, -0.15f, 0.45f, 0.0f, -0.15f, 0.45f, 0.0f, 0.15f,
		0.45f, 0.0f, 0.15f, 0.15f, 0.0f, 0.15f, 0.15f, 0.0f, -0.15f,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}


void Character::processKeyboard(int direction, float deltaTime)
{
	camera.processKeyboard(direction, deltaTime);
}

void Character::processMouseMovement(float xoffset, float yoffset)
{
	camera.processMouseMovement(xoffset, yoffset);
}

void Character::update(World &world, float deltaTime)
{
}

void Character::render(const Shader &shader)
{
	shader.use();
	shader.setMatrix4("model", glm::mat4(1.0f));

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Character::jump()
{
}

void Character::setFlying(bool flying)
{
	if (flying)
	{
		movementSpeed = flySpeed;
	}
	else
	{
		movementSpeed = walkSpeed;
	}
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
