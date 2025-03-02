#include "WorldManager.hpp"
#include "World.hpp"
#include "Utils.hpp"
#include "Cube.hpp"
#include "Database.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <functional>

// --- Implémentation de Chunk ---
Chunk::Chunk()
	: p(0), q(0), faces(0), sign_faces(0), dirty(0), miny(0), maxy(0),
	  buffer(0), sign_buffer(0)
{
	// Les maps seront initialisées lors d'initChunk.
}

// --- Fonctions du module WorldManager ---
namespace WorldManager {

int chunked(float x, const Config &config) {
	int chunkSize = config.getChunkSize();
	return static_cast<int>(std::floor(x / chunkSize));
}

Chunk* findChunk(std::vector<Chunk> &chunks, int p, int q) {
	for (size_t i = 0; i < chunks.size(); i++) {
		if (chunks[i].p == p && chunks[i].q == q) {
			return &chunks[i];
		}
	}
	return nullptr;
}

int highestBlock(const Chunk &chunk, int worldX, int worldZ) {
	int result = -1;
	const auto &data = chunk.map.getData();
	for (size_t i = 0; i < data.size(); i++) {
		const World::MapEntry &entry = data[i];
		if (entry.value == 0)
			continue;
		int x = entry.e.x + chunk.map.getDx();
		int y = entry.e.y + chunk.map.getDy();
		int z = entry.e.z + chunk.map.getDz();
		if (x == worldX && z == worldZ) {
			result = std::max(result, y);
		}
	}
	return result;
}

void dirtyChunk(Chunk &chunk) {
	chunk.dirty = 1;
}

void initChunk(Chunk &chunk, int p, int q, const Config &config) {
	chunk.p = p;
	chunk.q = q;
	chunk.faces = 0;
	chunk.sign_faces = 0;
	chunk.dirty = 1;
	int chunkSize = config.getChunkSize();
	// Initialisation des cartes avec un décalage (comme dans le code original)
	chunk.map = World::Map(p * chunkSize - 1, 0, q * chunkSize - 1, 0x7fff);
	chunk.lights = World::Map(p * chunkSize - 1, 0, q * chunkSize - 1, 0xf);
	chunk.signs = Utils::SignList();
	chunk.buffer = 0;
	chunk.sign_buffer = 0;
}

void loadChunk(Chunk &chunk, const Config &config, Database::DB* db) {
	int p = chunk.p;
	int q = chunk.q;
	// Utiliser World::createWorld pour remplir la map.
	auto map_set_func = [](int x, int y, int z, int w, void *arg) {
		World::Map *map = static_cast<World::Map*>(arg);
		map->set(x, y, z, w);
	};
	World::createWorld(config, p, q, map_set_func, &chunk.map);
	// Charger les blocs et lumières depuis la base de données.
	db->loadBlocks(chunk.map, p, q);
	db->loadLights(chunk.lights, p, q);
	db->loadSigns(chunk.signs, p, q);
}

// --- Fonctions de génération de géométrie du chunk ---
//
// Pour la génération, nous utilisons des macros locales définies en fonction du chunkSize.
namespace {
	// Ces macros dépendent du chunkSize
	inline int calc_XZ_SIZE(int chunkSize) { return chunkSize * 3 + 2; }
	inline int calc_XZ_LO(int chunkSize) { return chunkSize; }
	inline int calc_XZ_HI(int chunkSize) { return chunkSize * 2 + 1; }
	// Calcule l'indice dans un tableau 3D (x, y, z) pour une dimension XZ_SIZE et hauteur Y_SIZE.
	inline int XYZ(int x, int y, int z, int XZ_SIZE) {
		return y * XZ_SIZE * XZ_SIZE + x * XZ_SIZE + z;
	}
	inline int XZ(int x, int z, int XZ_SIZE) {
		return x * XZ_SIZE + z;
	}
}

// computeChunk : calcule la géométrie d'un chunk et remplit le WorkerItem.
void computeChunk(WorkerItem &item, const Config &config) {
	int chunkSize = config.getChunkSize();
	int XZ_SIZE = calc_XZ_SIZE(chunkSize);
	int XZ_LO = calc_XZ_LO(chunkSize);
	int XZ_HI = calc_XZ_HI(chunkSize);
	
	int totalXYZ = XZ_SIZE * XZ_SIZE * Y_SIZE;
	char *opaque = (char*)calloc(totalXYZ, sizeof(char));
	char *light = (char*)calloc(totalXYZ, sizeof(char));
	int totalXZ = XZ_SIZE * XZ_SIZE;
	char *highest = (char*)calloc(totalXZ, sizeof(char));

	// Origine du tableau, basée sur la map centrale.
	// On suppose que item.blockMap et item.lightMap sont initialisées (pointeurs vers les maps du chunk central).
	int ox = item.p * chunkSize - chunkSize - 1;
	// Remplissage de l'opacité
	const auto &data = item.blockMap->getData();
	for (size_t i = 0; i < data.size(); i++) {
		const World::MapEntry &entry = data[i];
		if (entry.value == 0)
			continue;
		int x = entry.e.x + item.blockMap->getDx();
		int y = entry.e.y + item.blockMap->getDy();
		int z = entry.e.z + item.blockMap->getDz();
		if (x < 0 || y < 0 || z < 0 || x >= XZ_SIZE || y >= Y_SIZE || z >= XZ_SIZE)
			continue;
		opaque[XYZ(x, y, z, XZ_SIZE)] = !World::isTransparent(entry.e.w);
		highest[XZ(x, z, XZ_SIZE)] = std::max(highest[XZ(x, z, XZ_SIZE)], (char)y);
	}
	
	// Fonction locale pour flood fill de la lumière.
	std::function<void(int, int, int, int, int)> lightFill = [&](int x, int y, int z, int w, int force) {
		if (x + w < XZ_LO || z + w < XZ_LO)
			return;
		if (x - w > XZ_HI || z - w > XZ_HI)
			return;
		if (y < 0 || y >= Y_SIZE)
			return;
		int idx = XYZ(x, y, z, XZ_SIZE);
		if (light[idx] >= w)
			return;
		if (!force && opaque[idx])
			return;
		light[idx] = w;
		w--;
		if (w < 0)
			return;
		lightFill(x - 1, y, z, w, 0);
		lightFill(x + 1, y, z, w, 0);
		lightFill(x, y - 1, z, w, 0);
		lightFill(x, y + 1, z, w, 0);
		lightFill(x, y, z - 1, w, 0);
		lightFill(x, y, z + 1, w, 0);
	};
	
	// Appliquer le flood fill pour la carte des lumières.
	const auto &lightData = item.lightMap->getData();
	for (size_t i = 0; i < lightData.size(); i++) {
		const World::MapEntry &entry = lightData[i];
		int x = entry.e.x + item.lightMap->getDx();
		int y = entry.e.y + item.lightMap->getDy();
		int z = entry.e.z + item.lightMap->getDz();
		lightFill(x, y, z, entry.e.w, 1);
	}
	
	// Calcul du nombre de faces exposées dans la map centrale.
	int miny = 256, maxy = 0, faces = 0;
	for (size_t i = 0; i < data.size(); i++) {
		const World::MapEntry &entry = data[i];
		if (entry.value == 0 || entry.e.w <= 0)
			continue;
		int x = entry.e.x + item.blockMap->getDx();
		int y = entry.e.y + item.blockMap->getDy();
		int z = entry.e.z + item.blockMap->getDz();
		int f1 = !opaque[XYZ(x - 1, y, z, XZ_SIZE)];
		int f2 = !opaque[XYZ(x + 1, y, z, XZ_SIZE)];
		int f3 = !opaque[XYZ(x, y + 1, z, XZ_SIZE)];
		int f4 = (y > 0) ? !opaque[XYZ(x, y - 1, z, XZ_SIZE)] : 0;
		int f5 = !opaque[XYZ(x, y, z - 1, XZ_SIZE)];
		int f6 = !opaque[XYZ(x, y, z + 1, XZ_SIZE)];
		int total = f1 + f2 + f3 + f4 + f5 + f6;
		if (total == 0)
			continue;
		if (World::isPlant(entry.e.w))
			total = 4;
		miny = std::min(miny, static_cast<int>(entry.e.y));
		maxy = std::max(maxy, static_cast<int>(entry.e.y));
		faces += total;
	}
	
	// Génération de la géométrie du chunk.
	// Allouer un tableau pour stocker les faces (chaque face : 10 floats, 6 vertices).
	GLfloat *geomData = Utils::malloc_faces(10, faces);
	int offset = 0;
	for (size_t i = 0; i < data.size(); i++) {
		const World::MapEntry &entry = data[i];
		if (entry.value == 0 || entry.e.w <= 0)
			continue;
		int x = entry.e.x + item.blockMap->getDx();
		int y = entry.e.y + item.blockMap->getDy();
		int z = entry.e.z + item.blockMap->getDz();
		int f1 = !opaque[XYZ(x - 1, y, z, XZ_SIZE)];
		int f2 = !opaque[XYZ(x + 1, y, z, XZ_SIZE)];
		int f3 = !opaque[XYZ(x, y + 1, z, XZ_SIZE)];
		int f4 = (y > 0) ? !opaque[XYZ(x, y - 1, z, XZ_SIZE)] : 0;
		int f5 = !opaque[XYZ(x, y, z - 1, XZ_SIZE)];
		int f6 = !opaque[XYZ(x, y, z + 1, XZ_SIZE)];
		int total = f1 + f2 + f3 + f4 + f5 + f6;
		if (total == 0)
			continue;
		// Pour simplifier, on utilise des matrices d'occlusion et de lumière constantes.
		float aoMat[6][4] = { {0.8f, 0.8f, 0.8f, 0.8f},
							  {0.8f, 0.8f, 0.8f, 0.8f},
							  {0.8f, 0.8f, 0.8f, 0.8f},
							  {0.8f, 0.8f, 0.8f, 0.8f},
							  {0.8f, 0.8f, 0.8f, 0.8f},
							  {0.8f, 0.8f, 0.8f, 0.8f} };
		float lightMat[6][4] = { {1, 1, 1, 1},
								 {1, 1, 1, 1},
								 {1, 1, 1, 1},
								 {1, 1, 1, 1},
								 {1, 1, 1, 1},
								 {1, 1, 1, 1} };
		if (World::isPlant(entry.e.w)) {
			total = 4;
			// Calculer une rotation aléatoire basée sur simplex2.
			float rotation = Cube::simplex2(static_cast<float>(x), static_cast<float>(z), 4, 0.5f, 2) * 360;
			Cube::makePlant(geomData + offset, 0.8f, 1.0f, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 0.5f, entry.e.w, rotation);
		} else {
			Cube::makeCube(geomData + offset, aoMat, lightMat,
						   f1, f2, f3, f4, f5, f6,
						   static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 0.5f, entry.e.w);
		}
		offset += total * 60; // Chaque face utilise 60 floats (4 faces de 15 floats par face, par exemple).
	}
	
	item.miny = miny;
	item.maxy = maxy;
	item.faces = faces;
	item.data = geomData;
	
	free(opaque);
	free(light);
	free(highest);
}

// Fonction locale pour générer le buffer des panneaux (signs) du chunk.
static GLuint genSignBuffer(Chunk &chunk) {
	int totalGlyphs = 0;
	const auto &signs = chunk.signs.getSigns();
	for (size_t i = 0; i < signs.size(); i++) {
		totalGlyphs += static_cast<int>(strlen(signs[i].text.c_str()));
	}
	if (totalGlyphs == 0)
		return 0;
	GLfloat *data = Utils::malloc_faces(5, totalGlyphs);
	// Pour simplifier, on ne génère pas de géométrie détaillée pour chaque glyphe.
	// On suppose ici que chaque glyphe occupe 6 vertices.
	int length = totalGlyphs;
	GLuint buffer = Utils::gen_faces(5, length, data);
	Utils::del_buffer(chunk.sign_buffer);
	chunk.sign_buffer = buffer;
	chunk.sign_faces = totalGlyphs;
	return buffer;
}

void generateChunk(Chunk &chunk, const WorkerItem &item, const Config &config) {
	chunk.miny = item.miny;
	chunk.maxy = item.maxy;
	chunk.faces = item.faces;
	Utils::del_buffer(chunk.buffer);
	chunk.buffer = Utils::gen_faces(10, item.faces, item.data);
	genSignBuffer(chunk);
	chunk.dirty = 0;
}

} // namespace WorldManager
