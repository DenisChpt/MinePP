/**
 * @class Player
 * @brief Représente le joueur et gère son comportement, sa physique et ses interactions avec le monde.
 *
 * @details La classe Player intègre une instance de Camera et interagit avec le monde via la vérification
 *          des collisions (MovementSimulation) et la mise à jour de la position en fonction de la gravité et des actions utilisateur.
 *          Elle gère également les actions de placement ou de suppression de blocs.
 */


#pragma once

#include "../Physics/MovementSimulation.hpp"
#include "../World/World.hpp"
#include "../Common.hpp"
#include "Camera.hpp"

class Player
{
	// The player is 0.6 blocks wide and 1.8 blocks tall, the eyes are approximately 0.3 units from the top

	Camera camera;

	Ref<World> world;
	Ref<Persistence> persistence;
	BlockData::BlockType blockToPlace = BlockData::BlockType::grass;

	glm::vec3 gravity{0};

	float movementSpeedMultiplier = 1;
	float mouseSensitivity = .5;

	float gravityConstant = DefaultGravity;
	float jumpHeightMultiplier = 1;
	bool canJump = false;

	bool isRunning = false;
	bool isSurvivalMovement = false;
	bool shouldResetMouse = true;

public:
	static constexpr AABB PlayerAABB = AABB{
		{-0.3, -1.5, -0.3},
		{0.3, 0.3, 0.3},
	};

	static constexpr std::array<glm::vec3, 8> PlayerBoundingBox = {{
		{0.3, 0.3, 0.3},
		{0.3, 0.3, -0.3},
		{-0.3, 0.3, 0.3},
		{-0.3, 0.3, -0.3},
		{0.3, -1.5, 0.3},
		{0.3, -1.5, -0.3},
		{-0.3, -1.5, 0.3},
		{-0.3, -1.5, -0.3},
	}};

	static constexpr float DefaultGravity = 46.62f;
	static constexpr float Reach = 4.5f;

	explicit Player(const Ref<World> &world, const Ref<Persistence> &persistence);
	~Player();

	void update(float deltaTime);

	[[nodiscard]] bool getIsSurvivalMovement() const { return isSurvivalMovement; };
	void setSurvivalMovement(bool isSurvival)
	{
		gravity = glm::vec3(0);
		isSurvivalMovement = isSurvival;
	};

	[[nodiscard]] const Camera &getCamera() const { return camera; };

	[[nodiscard]] float getJumpHeightMultiplier() const { return jumpHeightMultiplier; };
	void setJumpHeightMultiplier(float multiplier) { jumpHeightMultiplier = multiplier; };

	[[nodiscard]] float getGravityConstant() const { return gravityConstant; };
	void setGravityConstant(float constant) { gravityConstant = constant; };

	[[nodiscard]] float getMovementSpeedMultiplier() const { return movementSpeedMultiplier; };
	void setMovementSpeedMultiplier(float movementSpeed) { movementSpeedMultiplier = movementSpeed; }

	[[nodiscard]] float getJumpSpeed() const { return jumpHeightMultiplier * gravityConstant / 4.5f; };
	[[nodiscard]] float getWalkingSpeed() const { return movementSpeedMultiplier * 4.317f; };
	[[nodiscard]] float getRunningSpeed() const { return movementSpeedMultiplier * 5.612f; };

	[[nodiscard]] BlockData::BlockType getBlockToPlace() const { return blockToPlace; };
	void setBlockToPlace(BlockData::BlockType block) { blockToPlace = block; };

	void onKeyEvent(int32_t key, int32_t scancode, int32_t action, int32_t mode);
	void onMouseButtonEvent(int32_t button, int32_t action, int32_t mods);
	void onCursorPositionEvent(double d, double d1);

	void resetMousePosition();
};
