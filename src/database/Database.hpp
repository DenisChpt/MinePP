#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "sqlite3.h"

// Pour interagir avec les données du monde et les panneaux.
#include "Map.hpp"
#include "Sign.hpp"
#include "Ring.hpp"

namespace Database {

class DB {
public:
	DB();
	~DB();

	// Activation/désactivation globale de la persistance.
	void enable();
	void disable();
	bool isEnabled() const;

	// Initialisation et fermeture de la base de données.
	int init(const std::string &path);
	void close();
	void commit();

	// Fonctions d'authentification.
	void authSet(const std::string &username, const std::string &identityToken);
	int authSelect(const std::string &username);
	void authSelectNone();
	bool authGet(const std::string &username, std::string &identityToken);
	bool authGetSelected(std::string &username, std::string &identityToken);

	// Sauvegarde et chargement de l'état du monde.
	void saveState(float x, float y, float z, float rx, float ry);
	bool loadState(float &x, float &y, float &z, float &rx, float &ry);

	// Insertion dans la base.
	void insertBlock(int p, int q, int x, int y, int z, int w);
	void insertLight(int p, int q, int x, int y, int z, int w);
	void insertSign(int p, int q, int x, int y, int z, int face, const std::string &text);

	// Suppression.
	void deleteSign(int x, int y, int z, int face);
	void deleteSigns(int x, int y, int z);
	void deleteAllSigns();

	// Chargement depuis la base.
	void loadBlocks(World::Map &map, int p, int q);
	void loadLights(World::Map &map, int p, int q);
	void loadSigns(Utils::SignList &list, int p, int q);

	// Gestion de la « key ».
	int getKey(int p, int q);
	void setKey(int p, int q, int key);

	// Démarrage/arrêt du worker de traitement asynchrone.
	void workerStart();
	void workerStop();

private:
	// Fonction exécutée par le thread worker pour traiter les opérations du tampon.
	void workerRun();

	// Exécute un commit sur la base (commit + début d'une nouvelle transaction).
	void execCommit();
	void _db_insert_light(int p, int q, int x, int y, int z, int w);
	void _db_insert_block(int p, int q, int x, int y, int z, int w);
	void _db_set_key(int p, int q, int key);

	// Pointeurs vers la base SQLite et les requêtes préparées.
	sqlite3 *db_;
	sqlite3_stmt *insertBlockStmt_;
	sqlite3_stmt *insertLightStmt_;
	sqlite3_stmt *insertSignStmt_;
	sqlite3_stmt *deleteSignStmt_;
	sqlite3_stmt *deleteSignsStmt_;
	sqlite3_stmt *loadBlocksStmt_;
	sqlite3_stmt *loadLightsStmt_;
	sqlite3_stmt *loadSignsStmt_;
	sqlite3_stmt *getKeyStmt_;
	sqlite3_stmt *setKeyStmt_;

	std::atomic<bool> dbEnabled_;
	std::atomic<bool> workerRunning_;
	std::thread workerThread_;
	std::mutex mtx_;
	std::condition_variable cond_;
	std::mutex loadMtx_;

	// Tampon circulaire pour regrouper les opérations asynchrones.
	Utils::Ring ring_;
};

} // namespace Database

#endif // DATABASE_HPP
