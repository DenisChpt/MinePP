#include "NetworkManager.hpp"
#include "Client.hpp"    // Utilisation de la classe Network::Client
#include "Utils.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <chrono>

// Nous encapsulons un objet Network::Client dans ce module.
namespace NetworkManager {

static Network::Client* client = nullptr;
static bool connected = false;

bool init(const std::string &serverAddr, int serverPort) {
	if (!client) {
		client = new Network::Client();
	}
	client->enable();
	if (!client->connect(serverAddr, serverPort)) {
		std::cerr << "[NetworkManager] Échec de la connexion au serveur " 
				  << serverAddr << ":" << serverPort << std::endl;
		connected = false;
		return false;
	}
	connected = true;
	return true;
}

void start() {
	if (client && connected) {
		client->start();
	}
}

void stop() {
	if (client) {
		client->stop();
		client->disable();
		connected = false;
		delete client;
		client = nullptr;
	}
}

void sendVersion(int version) {
	if (client && connected) {
		client->version(version);
	}
}

void sendLogin(const std::string &username, const std::string &accessToken) {
	if (client && connected) {
		client->login(username, accessToken);
	}
}

void sendPosition(float x, float y, float z, float rx, float ry) {
	if (client && connected) {
		client->position(x, y, z, rx, ry);
	}
}

void sendMessage(const std::string &message) {
	if (client && connected) {
		client->send(message);
	}
}

// Fonction utilitaire pour parser une ligne de message reçue.
// Exemple de message de mise à jour du joueur : "U,123,10.0,64.0,20.0,1.57,0.0"
static void parseMessage(const std::string &line, std::vector<Player> &players) {
	if (line.empty())
		return;

	// Traitement du message de mise à jour du joueur (identifié par 'U')
	if (line[0] == 'U') {
		int pid;
		float ux, uy, uz, urx, ury;
		// Format attendu : U,playerId,ux,uy,uz,urx,ury
		if (sscanf(line.c_str(), "U,%d,%f,%f,%f,%f,%f", &pid, &ux, &uy, &uz, &urx, &ury) == 6) {
			// Rechercher le joueur par ID dans le vecteur players
			Player* player = nullptr;
			for (auto &p : players) {
				if (p.id == pid) {
					player = &p;
					break;
				}
			}
			if (player) {
				// Mise à jour avec interpolation : sauvegarder l'état précédent et mettre à jour l'état courant.
				player->previous = player->current;
				player->current.x = ux;
				player->current.y = uy;
				player->current.z = uz;
				player->current.rx = urx;
				player->current.ry = ury;
				// Utilisation du temps système pour l'horodatage
				using namespace std::chrono;
				player->current.t = duration_cast<duration<float>>(system_clock::now().time_since_epoch()).count();
			} else {
				std::cout << "[NetworkManager] Joueur d'ID " << pid << " non trouvé dans le vecteur." << std::endl;
			}
		}
	}
	// D'autres types de messages (B, L, P, D, K, R, E, T, etc.) pourront être traités ici.
}

void update(std::vector<Player> &players) {
	if (client && connected) {
		std::string received = client->recv();
		if (!received.empty()) {
			std::istringstream iss(received);
			std::string line;
			while (std::getline(iss, line)) {
				parseMessage(line, players);
			}
		}
	}
}

bool isConnected() {
	return connected;
}

} // namespace NetworkManager
