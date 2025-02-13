#pragma once
#include "UIElement.hpp"

namespace Game {

class HUD : public UIElement {
public:
	HUD();
	virtual ~HUD();

	void update(float deltaTime) override;
	void render() override;
};

} // namespace Game
