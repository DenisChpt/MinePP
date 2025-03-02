#include "DatabaseManager.hpp"
#include <iostream>
#include <cstdlib>

namespace DatabaseManager {

static Database::DB* db = nullptr;

bool init(const std::string &dbPath) {
	if (!db) {
		db = new Database::DB();
	}
	db->enable();
	int rc = db->init(dbPath);
	if (rc != 0) {
		std::cerr << "[DatabaseManager] Erreur lors de l'initialisation de la BDD: " << rc << std::endl;
		return false;
	}
	std::cout << "[DatabaseManager] Base de données initialisée avec succès: " << dbPath << std::endl;
	return true;
}

void commit() {
	if (db) {
		db->commit();
	}
}

void close() {
	if (db) {
		db->close();
		db->disable();
		delete db;
		db = nullptr;
	}
}

void enable() {
	if (db) {
		db->enable();
	}
}

void disable() {
	if (db) {
		db->disable();
	}
}

void savePlayerState(float x, float y, float z, float rx, float ry) {
	if (db) {
		db->saveState(x, y, z, rx, ry);
		std::cout << "[DatabaseManager] État du joueur sauvegardé." << std::endl;
	}
}

bool loadPlayerState(float &x, float &y, float &z, float &rx, float &ry) {
	if (db) {
		bool loaded = db->loadState(x, y, z, rx, ry);
		if (loaded) {
			std::cout << "[DatabaseManager] État du joueur chargé." << std::endl;
		}
		return loaded;
	}
	return false;
}

void updateBlock(int p, int q, int x, int y, int z, int w) {
	if (db) {
		db->insertBlock(p, q, x, y, z, w);
		std::cout << "[DatabaseManager] Bloc mis à jour dans le chunk (" << p << ", " << q << ")." << std::endl;
	}
}

void updateLight(int p, int q, int x, int y, int z, int w) {
	if (db) {
		db->insertLight(p, q, x, y, z, w);
		std::cout << "[DatabaseManager] Lumière mise à jour dans le chunk (" << p << ", " << q << ")." << std::endl;
	}
}

void updateSign(int p, int q, int x, int y, int z, int face, const std::string &text) {
	if (db) {
		db->insertSign(p, q, x, y, z, face, text);
		std::cout << "[DatabaseManager] Panneau mis à jour dans le chunk (" << p << ", " << q << ")." << std::endl;
	}
}

void loadChunkBlocks(World::Map &map, int p, int q) {
	if (db) {
		db->loadBlocks(map, p, q);
		std::cout << "[DatabaseManager] Blocs chargés pour le chunk (" << p << ", " << q << ")." << std::endl;
	}
}

void loadChunkLights(World::Map &map, int p, int q) {
	if (db) {
		db->loadLights(map, p, q);
		std::cout << "[DatabaseManager] Lumières chargées pour le chunk (" << p << ", " << q << ")." << std::endl;
	}
}

void loadChunkSigns(Utils::SignList &signs, int p, int q) {
	if (db) {
		db->loadSigns(signs, p, q);
		std::cout << "[DatabaseManager] Panneaux chargés pour le chunk (" << p << ", " << q << ")." << std::endl;
	}
}

// Fonctions d'authentification
void authSet(const std::string &username, const std::string &identityToken) {
	if (db) {
		db->authSet(username, identityToken);
		std::cout << "[DatabaseManager] Identifiant enregistré pour " << username << std::endl;
	}
}

int authSelect(const std::string &username) {
	if (db) {
		return db->authSelect(username);
	}
	return 0;
}

bool authGet(const std::string &username, std::string &identityToken) {
	if (db) {
		return db->authGet(username, identityToken);
	}
	return false;
}

bool authGetSelected(std::string &username, std::string &identityToken) {
	if (db) {
		return db->authGetSelected(username, identityToken);
	}
	return false;
}

} // namespace DatabaseManager
