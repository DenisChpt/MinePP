#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <glm/glm.hpp>
#include "Camera.hpp"
#include "World.hpp"
#include "Texture.hpp"

class Character
{
public:
	Character(const glm::vec3 &position, const glm::vec3 &up, float yaw, float pitch);

	void processKeyboard(int direction, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset);
	void update(World &world, float deltaTime);
	void render(const Shader &shader);

	void jump();
	void setFlying(bool flying);
	bool getFlying() const;
	void setupMesh();

	glm::vec3 getPosition() const;
	glm::mat4 getViewMatrix() const;

private:
	glm::vec3 position;
	glm::vec3 size;
	Camera camera;
	// Texture texture;
	float verticalVelocity;
	bool isFlying;
	bool canGoUp;
	bool canGoDown;
	float movementSpeed;
	float walkSpeed;
	float flySpeed;
	GLuint VAO, VBO;
};

#endif // CHARACTER_HPP
