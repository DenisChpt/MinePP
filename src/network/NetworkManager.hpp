#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <string>
#include <vector>
#include "PlayerManager.hpp"
#include "Config.hpp"

// Le module NetworkManager encapsule la communication réseau entre le client et le serveur.
namespace NetworkManager {

	/// Initialise la connexion au serveur avec l'adresse et le port spécifiés.
	/// Retourne true en cas de succès, false sinon.
	bool init(const std::string &serverAddr, int serverPort);

	/// Démarre le client réseau (active le thread de réception).
	void start();

	/// Arrête la communication et ferme la connexion.
	void stop();

	/// Envoie la version du protocole au serveur.
	void sendVersion(int version);

	/// Envoie une demande de login avec les identifiants fournis.
	void sendLogin(const std::string &username, const std::string &accessToken);

	/// Envoie la position et la rotation du joueur.
	void sendPosition(float x, float y, float z, float rx, float ry);

	/// Envoie un message générique au serveur.
	void sendMessage(const std::string &message);

	/// Met à jour l'état du réseau en traitant les messages reçus.
	/// Cette fonction met à jour le vecteur players (via PlayerManager) en fonction des messages reçus.
	void update(std::vector<Player> &players);

	/// Retourne true si le client est connecté, false sinon.
	bool isConnected();

} // namespace NetworkManager

#endif // NETWORK_MANAGER_HPP
