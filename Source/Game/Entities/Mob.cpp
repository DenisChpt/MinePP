#include "Mob.hpp"
#include <iostream>

namespace Game {

Mob::Mob(const glm::vec3& position)
	: m_Position(position)
{
}

Mob::~Mob() {
}

void Mob::update(float deltaTime) {
	std::cout << "[Mob] Update (" << deltaTime << "s)." << std::endl;
}

void Mob::render() {
	std::cout << "[Mob] Render at (" << m_Position.x << ", " << m_Position.y << ", " << m_Position.z << ")." << std::endl;
}

} // namespace Game
