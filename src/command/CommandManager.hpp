#ifndef COMMAND_MANAGER_HPP
#define COMMAND_MANAGER_HPP

#include <string>
#include <vector>
#include "PlayerManager.hpp"
#include "WorldManager.hpp"
#include "Config.hpp"
#include "Database.hpp"

// Namespace pour gérer les commandes textuelles saisies par l'utilisateur.
namespace CommandManager {

	/**
	 * Exécute la commande textuelle 'command' et effectue les actions correspondantes.
	 *
	 * Paramètres :
	 * - command         : chaîne contenant la commande saisie (ex. "/tp 10 64 20").
	 * - currentPlayerId : l'ID du joueur qui a saisi la commande.
	 * - players         : vecteur de joueurs géré par PlayerManager.
	 * - chunks          : vecteur de chunks géré par WorldManager.
	 * - config          : configuration du jeu.
	 * - db              : pointeur vers l'instance Database::DB (pour sauvegarder les modifications du monde).
	 *
	 * Retourne true si la commande a été reconnue et exécutée, false sinon.
	 */
	bool executeCommand(const std::string &command,
						int currentPlayerId,
						std::vector<Player> &players,
						std::vector<Chunk> &chunks,
						const Config &config,
						Database::DB* db);

} // namespace CommandManager

#endif // COMMAND_MANAGER_HPP
