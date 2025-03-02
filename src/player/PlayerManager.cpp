#include "PlayerManager.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace PlayerManager {

void createPlayer(std::vector<Player>& players, int id, const std::string &name, const PlayerState &state) {
	Player newPlayer;
	newPlayer.id = id;
	newPlayer.name = name;
	newPlayer.current = state;
	newPlayer.previous = state;
	newPlayer.buffer = 0; // Le buffer sera initialisé par RenderManager lors du rendu.
	players.push_back(newPlayer);
}

Player* findPlayer(std::vector<Player>& players, int id) {
	for (auto &player : players) {
		if (player.id == id) {
			return &player;
		}
	}
	return nullptr;
}

void updatePlayer(std::vector<Player>& players, int id, const PlayerState &newState, bool interpolate) {
	Player* player = findPlayer(players, id);
	if (!player) {
		std::cerr << "[PlayerManager] Erreur: joueur d'ID " << id << " non trouvé pour mise à jour." << std::endl;
		return;
	}
	if (interpolate) {
		// Conserver l'état précédent pour l'interpolation
		player->previous = player->current;
		player->current = newState;
	} else {
		player->current = newState;
		player->previous = newState;
	}
	// Note: La mise à jour du buffer OpenGL pourra être gérée par RenderManager.
}

void interpolatePlayer(Player &player, float currentTime) {
	float dt = player.current.t - player.previous.t;
	if (dt <= 0.0f) {
		// Aucun intervalle pour interpoler, on garde l'état courant.
		return;
	}
	float factor = (currentTime - player.current.t) / dt;
	factor = std::min(std::max(factor, 0.0f), 1.0f);
	PlayerState interp;
	interp.x = player.previous.x + (player.current.x - player.previous.x) * factor;
	interp.y = player.previous.y + (player.current.y - player.previous.y) * factor;
	interp.z = player.previous.z + (player.current.z - player.previous.z) * factor;
	interp.rx = player.previous.rx + (player.current.rx - player.previous.rx) * factor;
	interp.ry = player.previous.ry + (player.current.ry - player.previous.ry) * factor;
	interp.t = currentTime;
	// Met à jour l'état courant pour le rendu interpolé.
	player.current = interp;
}

void deletePlayer(std::vector<Player>& players, int id) {
	auto it = std::remove_if(players.begin(), players.end(), [id](const Player &p) {
		return p.id == id;
	});
	if (it != players.end()) {
		players.erase(it, players.end());
	}
}

} // namespace PlayerManager
