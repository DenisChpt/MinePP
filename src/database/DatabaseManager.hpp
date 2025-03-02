#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <string>
#include "Database.hpp"
#include "World.hpp"
#include "Sign.hpp"

namespace DatabaseManager {

	/// Initialise la connexion à la base de données avec le chemin fourni.
	/// Retourne true en cas de succès, false sinon.
	bool init(const std::string &dbPath);

	/// Enregistre (commit) les transactions en attente.
	void commit();

	/// Ferme la connexion à la base de données et libère les ressources.
	void close();

	/// Active ou désactive la persistance (base de données).
	void enable();
	void disable();

	/// Sauvegarde l'état du joueur (position et rotation) dans la BDD.
	void savePlayerState(float x, float y, float z, float rx, float ry);

	/// Charge l'état du joueur depuis la BDD.
	/// Retourne true si un état a été chargé, false sinon.
	bool loadPlayerState(float &x, float &y, float &z, float &rx, float &ry);

	/// Met à jour un bloc dans la base de données.
	void updateBlock(int p, int q, int x, int y, int z, int w);

	/// Met à jour une source de lumière dans la BDD.
	void updateLight(int p, int q, int x, int y, int z, int w);

	/// Met à jour un panneau (sign) dans la BDD.
	void updateSign(int p, int q, int x, int y, int z, int face, const std::string &text);

	/// Charge les blocs d’un chunk depuis la BDD dans la map fournie.
	void loadChunkBlocks(World::Map &map, int p, int q);

	/// Charge les lumières d’un chunk depuis la BDD dans la map fournie.
	void loadChunkLights(World::Map &map, int p, int q);

	/// Charge les panneaux d’un chunk depuis la BDD dans la liste fournie.
	void loadChunkSigns(Utils::SignList &signs, int p, int q);

	// Fonctions d'authentification (déjà utilisées dans CommandManager par exemple)
	void authSet(const std::string &username, const std::string &identityToken);
	int authSelect(const std::string &username);
	bool authGet(const std::string &username, std::string &identityToken);
	bool authGetSelected(std::string &username, std::string &identityToken);

} // namespace DatabaseManager

#endif // DATABASE_MANAGER_HPP
