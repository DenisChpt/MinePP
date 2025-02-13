#pragma once

namespace Game {

class IUIElement {
public:
	virtual ~IUIElement() = default;
	virtual void update(float deltaTime) = 0;
	virtual void render() = 0;
};

} // namespace Game
