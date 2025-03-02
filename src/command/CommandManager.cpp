#include "CommandManager.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <chrono>

// Fonction utilitaire pour obtenir l'heure actuelle en secondes.
static float getCurrentTime() {
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	auto seconds = duration_cast<duration<float>>(now.time_since_epoch());
	return seconds.count();
}

namespace CommandManager {

bool executeCommand(const std::string &command,
					int currentPlayerId,
					std::vector<Player> &players,
					std::vector<Chunk> &chunks,
					const Config &config,
					Database::DB* db) {
	std::istringstream iss(command);
	std::string token;
	iss >> token;
	if (token.empty())
		return false;

	if (token == "/tp") {
		// /tp x y z : téléporte le joueur courant aux coordonnées spécifiées.
		float x, y, z;
		if (iss >> x >> y >> z) {
			Player* player = PlayerManager::findPlayer(players, currentPlayerId);
			if (player) {
				PlayerState newState = player->current;
				newState.x = x;
				newState.y = y;
				newState.z = z;
				newState.t = getCurrentTime();
				PlayerManager::updatePlayer(players, currentPlayerId, newState, false);
				std::cout << "[CommandManager] Téléportation du joueur " << currentPlayerId 
						  << " vers (" << x << ", " << y << ", " << z << ")." << std::endl;
			} else {
				std::cerr << "[CommandManager] Joueur " << currentPlayerId << " non trouvé." << std::endl;
			}
			return true;
		} else {
			std::cerr << "[CommandManager] Usage: /tp x y z" << std::endl;
			return false;
		}
	}
	else if (token == "/give") {
		// /give item : donne un item au joueur courant.
		std::string itemName;
		if (iss >> itemName) {
			// Pour cet exemple, nous affichons simplement un message.
			std::cout << "[CommandManager] Donner l'item \"" << itemName 
					  << "\" au joueur " << currentPlayerId << "." << std::endl;
			// Intégration possible avec un module InventoryManager.
			return true;
		} else {
			std::cerr << "[CommandManager] Usage: /give item" << std::endl;
			return false;
		}
	}
	else if (token == "/kick") {
		// /kick player : expulse le joueur dont le nom est donné.
		std::string playerName;
		if (iss >> playerName) {
			auto it = std::find_if(players.begin(), players.end(), [&](const Player &p) {
				return p.name == playerName;
			});
			if (it != players.end()) {
				int kickedId = it->id;
				PlayerManager::deletePlayer(players, kickedId);
				std::cout << "[CommandManager] Joueur \"" << playerName << "\" (ID " << kickedId 
						  << ") expulsé." << std::endl;
				// Optionnel : appeler NetworkManager pour notifier l'expulsion.
				return true;
			} else {
				std::cerr << "[CommandManager] Joueur \"" << playerName << "\" non trouvé." << std::endl;
				return false;
			}
		} else {
			std::cerr << "[CommandManager] Usage: /kick playerName" << std::endl;
			return false;
		}
	}
	else if (token == "/setblock") {
		// /setblock x y z type : modifie le bloc à la position donnée.
		int bx, by, bz, type;
		if (iss >> bx >> by >> bz >> type) {
			int chunkSize = config.getChunkSize();
			int cp = static_cast<int>(std::floor(bx / static_cast<float>(chunkSize)));
			int cq = static_cast<int>(std::floor(bz / static_cast<float>(chunkSize)));
			Chunk* chunk = nullptr;
			for (auto &ch : chunks) {
				if (ch.p == cp && ch.q == cq) {
					chunk = &ch;
					break;
				}
			}
			if (chunk) {
				// Mise à jour du bloc dans la map du chunk.
				chunk->map.set(bx, by, bz, type);
				if (db) {
					db->insertBlock(cp, cq, bx, by, bz, type);
				}
				// Marquer le chunk comme modifié pour que le WorkerManager le regénère.
				chunk->dirty = 1;
				std::cout << "[CommandManager] Bloc modifié en (" << bx << ", " << by << ", " << bz 
						  << ") -> type " << type << "." << std::endl;
				return true;
			} else {
				std::cerr << "[CommandManager] Chunk non trouvé pour les coordonnées (" 
						  << bx << ", " << by << ", " << bz << ")." << std::endl;
				return false;
			}
		} else {
			std::cerr << "[CommandManager] Usage: /setblock x y z type" << std::endl;
			return false;
		}
	}
	else {
		std::cerr << "[CommandManager] Commande inconnue: " << token << std::endl;
		return false;
	}
}

} // namespace CommandManager
