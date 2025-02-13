#pragma once
#include "IUIElement.hpp"

namespace Game {

class UIElement : public IUIElement {
public:
	UIElement();
	virtual ~UIElement();

	void update(float deltaTime) override;
	void render() override;
};

} // namespace Game
