#include "Player.hpp"

#include "../Math/Math.hpp"
#include "../Math/Math.inl"
#include "Camera.hpp"

Player::Player(const Ref<World> &world, const Ref<Persistence> &persistence)
	: persistence(persistence),
	  world(world)
{
	// Initialize from persistence camera
	const auto& savedCamera = persistence->getCamera();
	position = savedCamera.getPosition();
	yaw = savedCamera.getYaw();
	pitch = savedCamera.getPitch();
	updateCameraOrientation(yaw, pitch);
}

Player::~Player()
{
	// Create a Camera object for persistence
	Camera tempCam;
	tempCam.setPosition(position);
	tempCam.updateCameraOrientation(yaw, pitch);
	persistence->commitCamera(tempCam);
}

void Player::update(float deltaTime)
{
	gravity += glm::vec3(0, -1, 0) * gravityConstant * deltaTime;

	glm::vec3 moveDirection = getMoveDirection();

	canJump = false;
	glm::vec3 movement(0);
	if (glm::length(moveDirection) > 0)
	{
		float movementSpeed = isRunning ? getRunningSpeed() : getWalkingSpeed();
		movement = glm::normalize(moveDirection) * movementSpeed * deltaTime;
	}

	glm::vec3 currentPosition = position;

	if (isSurvivalMovement)
	{
		std::array<glm::vec3, 3> axes = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};

		for (const auto &axis : axes)
		{
			glm::vec3 movementInAxis = movement * axis;
			if (MovementSimulation::canMove(currentPosition, currentPosition + movementInAxis, *world))
			{
				currentPosition += movementInAxis;
			}
		}

		glm::vec3 positionWithGravity = currentPosition + gravity * deltaTime;
		if (MovementSimulation::canMove(currentPosition, positionWithGravity, *world))
		{
			currentPosition = positionWithGravity;
		}
		else
		{
			canJump = true;
			gravity = glm::vec3(0);
		}
	}
	else
	{
		currentPosition += movement;
	}

	setPosition(currentPosition);
}

void Player::onKeyEvent(int32_t key, int32_t, int32_t action, int32_t)
{
	if (action == GLFW_REPEAT)
	{
		return; // don't respond to repeatedly pressed buttons
	}

	bool isButtonPressed = action == GLFW_PRESS;

	if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
	{
		setIsMovingForward(isButtonPressed);
	}
	else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
	{
		setIsMovingBackward(isButtonPressed);
	}
	else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)
	{
		setIsMovingLeft(isButtonPressed);
	}
	else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT)
	{
		setIsMovingRight(isButtonPressed);
	}
	else if (key == GLFW_KEY_SPACE)
	{
		if (isSurvivalMovement)
		{
			setIsMovingUp(false);
			if (canJump && isButtonPressed)
			{
				gravity = glm::vec3(0, getJumpSpeed(), 0);
			}
		}
		else
		{
			setIsMovingUp(isButtonPressed);
		}
	}
	else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
	{
		if (isSurvivalMovement)
		{
			setIsMovingDown(false);
		}
		else
		{
			setIsMovingDown(isButtonPressed);
		}
	}
	else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		isRunning = isButtonPressed;
	}
}

void Player::onMouseButtonEvent(int32_t button, int32_t action, int32_t)
{
	if (action != GLFW_PRESS)
	{
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (WorldRayCast ray{position, lookDirection, *world, Reach})
		{
			world->placeBlock(BlockData::BlockType::air, ray.getHitTarget().position);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		WorldRayCast ray{position, lookDirection, *world, Reach};
		if (ray && ray.getHitTarget().hasNeighbor)
		{
			world->placeBlock(blockToPlace, ray.getHitTarget().neighbor);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (WorldRayCast ray{position, lookDirection, *world, Reach})
		{
			blockToPlace = ray.getHitTarget().block->type;
		}
	}
}

void Player::onCursorPositionEvent(double x, double y)
{
	static double lastX = x;
	static double lastY = y;

	if (shouldResetMouse)
	{
		shouldResetMouse = false;
		lastX = x;
		lastY = y;
	}

	float newYaw = yaw + static_cast<float>(-lastX + x) * mouseSensitivity;
	float newPitch = glm::clamp(pitch + static_cast<float>(lastY - y) * mouseSensitivity, -89.0f, 89.0f);
	updateCameraOrientation(newYaw, newPitch);

	lastX = x;
	lastY = y;
}

void Player::resetMousePosition()
{
	shouldResetMouse = true;
}

// Camera methods implementation
const glm::mat4 &Player::updateView()
{
	return view = calcView();
}

const glm::mat4 &Player::lookAt(glm::vec3 eye, glm::vec3 center)
{
	position = eye;
	updateCameraDirection(center);
	return updateView();
}

const glm::mat4 &Player::setPosition(glm::vec3 eye)
{
	position = eye;
	return updateView();
}

void Player::updateCameraDirection(glm::vec3 newForward)
{
	lookDirection = newForward;
	newForward.y = 0;
	forward.direction = glm::normalize(newForward);
	backward.direction = -newForward;

	right.direction = glm::normalize(glm::cross(newForward, cameraUp));
	left.direction = -right.direction;
}

void Player::updateCameraOrientation(float newYaw, float newPitch)
{
	yaw = newYaw;
	pitch = newPitch;
	updateCameraDirection(glm::normalize(glm::vec3{
		glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
		glm::sin(glm::radians(pitch)),
		glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
	}));

	updateView();
}

glm::mat4 Player::calcView() const
{
	return glm::lookAt(position, position + lookDirection, cameraUp);
}

glm::vec3 Player::getMoveDirection()
{
	auto moveDirection = glm::vec3(0);

	std::array<MovementDirection *, 6> directions = {
		&forward,
		&backward,
		&left,
		&right,
		&up,
		&down,
	};

	for (const auto direction : directions)
	{
		if (!direction->isMoving)
		{
			continue;
		}

		moveDirection += direction->direction;
	}

	return moveDirection;
}
