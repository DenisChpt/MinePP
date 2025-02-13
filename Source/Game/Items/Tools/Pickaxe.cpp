#include "Pickaxe.hpp"
#include <iostream>

namespace Game {

Pickaxe::Pickaxe()
	: Item("Pickaxe")
{
}

Pickaxe::~Pickaxe() {
}

void Pickaxe::use() {
	std::cout << "[Pickaxe] Swinging the pickaxe!" << std::endl;
}

} // namespace Game
