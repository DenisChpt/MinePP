#include "HUD.hpp"
#include <iostream>

namespace Game {

HUD::HUD() {
}

HUD::~HUD() {
}

void HUD::update(float deltaTime) {
	std::cout << "[HUD] Mise à jour du HUD (" << deltaTime << "s)." << std::endl;
}

void HUD::render() {
	std::cout << "[HUD] Rendu du HUD." << std::endl;
}

} // namespace Game
