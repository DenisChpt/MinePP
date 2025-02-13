#include "UIElement.hpp"
#include <iostream>

namespace Game {

UIElement::UIElement() {
}

UIElement::~UIElement() {
}

void UIElement::update(float deltaTime) {
	std::cout << "[UIElement] Mise à jour (" << deltaTime << "s)." << std::endl;
}

void UIElement::render() {
	std::cout << "[UIElement] Rendu." << std::endl;
}

} // namespace Game
