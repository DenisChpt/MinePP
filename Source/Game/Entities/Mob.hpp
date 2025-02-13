#pragma once
#include <glm/glm.hpp>

namespace Game {

class Mob {
public:
	Mob(const glm::vec3& position);
	virtual ~Mob();

	virtual void update(float deltaTime);
	virtual void render();

protected:
	glm::vec3 m_Position;
};

} // namespace Game
