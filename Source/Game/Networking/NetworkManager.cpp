#include "NetworkManager.hpp"
#include <iostream>

namespace Game {

NetworkManager::NetworkManager() {
}

NetworkManager::~NetworkManager() {
}

bool NetworkManager::initialize() {
	std::cout << "[NetworkManager] Initialisé." << std::endl;
	return true;
}

void NetworkManager::update(float deltaTime) {
	std::cout << "[NetworkManager] Mise à jour (" << deltaTime << "s)." << std::endl;
}

void NetworkManager::shutdown() {
	std::cout << "[NetworkManager] Arrêt du système réseau." << std::endl;
}

} // namespace Game
