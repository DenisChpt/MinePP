#ifndef WORLD_MANAGER_HPP
#define WORLD_MANAGER_HPP

#include <vector>
#include <GL/glew.h>
#include "Config.hpp"
#include "World.hpp"
#include "Utils.hpp"
#include "Cube.hpp"
#include "Database.hpp"
#include "Sign.hpp"

// Structure représentant un chunk du monde.
struct Chunk {
	World::Map map;          // Carte des blocs
	World::Map lights;       // Carte des lumières
	Utils::SignList signs;   // Liste des panneaux (signs)
	int p;                   // Coordonnée de chunk en X (indice)
	int q;                   // Coordonnée de chunk en Z (indice)
	int faces;               // Nombre de faces générées (pour le rendu)
	int sign_faces;          // Nombre de faces de panneaux
	int dirty;               // Indique si le chunk a été modifié et doit être regénéré
	int miny, maxy;          // Hauteur minimale et maximale dans le chunk
	GLuint buffer;           // Buffer OpenGL pour la géométrie du chunk
	GLuint sign_buffer;      // Buffer OpenGL pour les panneaux
	Chunk();
};

namespace WorldManager {

	// --- Fonctions de conversion ---
	/// Convertit une position mondiale (x ou z) en indice de chunk, selon la taille définie dans Config.
	int chunked(float x, const Config &config);

	// --- Fonctions de recherche et de marquage ---
	/// Recherche un chunk ayant les indices (p, q) dans le vecteur passé.
	Chunk* findChunk(std::vector<Chunk> &chunks, int p, int q);

	/// Retourne la hauteur du bloc le plus haut pour des coordonnées mondiales (x, z) dans le chunk.
	int highestBlock(const Chunk &chunk, int worldX, int worldZ);

	/// Marque le chunk comme "dirty", indiquant qu'il doit être regénéré.
	void dirtyChunk(Chunk &chunk);

	// --- Fonctions d'initialisation et de chargement ---
	/// Initialise un chunk en créant ses cartes (map et lights) et en réinitialisant ses indicateurs.
	void initChunk(Chunk &chunk, int p, int q, const Config &config);

	/// Charge un chunk en générant le monde via World::createWorld et en chargeant les blocs et lumières depuis la BDD.
	void loadChunk(Chunk &chunk, const Config &config, Database::DB* db);

	// --- Génération de géométrie du chunk ---
	/// Structure utilisée par le worker pour générer un chunk.
	struct WorkerItem {
		int p, q;              // Coordonnées du chunk
		// Pointeurs vers les cartes de blocs et lumières (généralement la map centrale du chunk)
		World::Map* blockMap;
		World::Map* lightMap;
		int miny;              // Hauteur minimale détectée
		int maxy;              // Hauteur maximale détectée
		int faces;             // Nombre total de faces générées
		GLfloat* data;         // Tableau de géométrie (prêt à être envoyé dans un buffer OpenGL)
	};

	/// Calcule la géométrie d’un chunk et remplit le WorkerItem.
	void computeChunk(WorkerItem &item, const Config &config);

	/// Génère le buffer OpenGL du chunk à partir des données calculées dans le WorkerItem.
	void generateChunk(Chunk &chunk, const WorkerItem &item, const Config &config);

} // namespace WorldManager

#endif // WORLD_MANAGER_HPP
