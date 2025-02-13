#include "AtlasGenerator.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <fstream>
#include <GL/glew.h>
#include <json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using json = nlohmann::json;

// Structure pour représenter un rectangle (région) dans l'atlas.
struct Rect {
	int x;
	int y;
	int width;
	int height;
};

static inline int rectArea(const Rect& r) {
	return r.width * r.height;
}

static bool rectContains(const Rect& a, const Rect& b) {
	return b.x >= a.x && b.y >= a.y &&
		   (b.x + b.width) <= (a.x + a.width) &&
		   (b.y + b.height) <= (a.y + a.height);
}

/// Classe interne pour le bin packing avec l'algorithme MaxRects.
class MaxRectsBinPack {
public:
	MaxRectsBinPack(int width, int height)
		: binWidth(width), binHeight(height)
	{
		// Commencer avec un rectangle libre couvrant l'intégralité de la zone.
		Rect initial = {0, 0, width, height};
		freeRectangles.push_back(initial);
	}

	/// Tente d'insérer un rectangle de dimensions (width, height)
	/// en utilisant la heuristique Best Area Fit.
	/// Si réussi, retourne le rectangle placé dans outRect.
	bool Insert(int width, int height, Rect& outRect) {
		int bestAreaFit = INT_MAX;
		int bestShortSideFit = INT_MAX;
		bool found = false;
		Rect bestNode = {0, 0, 0, 0};

		for (const Rect &freeRect : freeRectangles) {
			if (freeRect.width >= width && freeRect.height >= height) {
				int areaFit = freeRect.width * freeRect.height - width * height;
				int shortSideFit = std::min(freeRect.width - width, freeRect.height - height);
				if (areaFit < bestAreaFit ||
					(areaFit == bestAreaFit && shortSideFit < bestShortSideFit)) {
					bestNode.x = freeRect.x;
					bestNode.y = freeRect.y;
					bestNode.width = width;
					bestNode.height = height;
					bestAreaFit = areaFit;
					bestShortSideFit = shortSideFit;
					found = true;
				}
			}
		}
		if (!found)
			return false;

		outRect = bestNode;
		SplitFreeRectangles(outRect);
		PruneFreeList();
		return true;
	}

private:
	int binWidth;
	int binHeight;
	std::vector<Rect> freeRectangles;

	// Découpe les rectangles libres qui intersectent avec le rectangle utilisé.
	void SplitFreeRectangles(const Rect& usedRect) {
		std::vector<Rect> newFreeRects;
		for (size_t i = 0; i < freeRectangles.size(); ) {
			Rect freeRect = freeRectangles[i];
			if (!DoRectanglesIntersect(freeRect, usedRect)) {
				++i;
				continue;
			}
			// Si les rectangles se chevauchent, découper freeRect en jusqu'à 4 sous-rectangles.
			if (usedRect.x > freeRect.x && usedRect.x < freeRect.x + freeRect.width) {
				Rect newRect = freeRect;
				newRect.width = usedRect.x - newRect.x;
				newFreeRects.push_back(newRect);
			}
			if (usedRect.x + usedRect.width < freeRect.x + freeRect.width) {
				Rect newRect = freeRect;
				newRect.x = usedRect.x + usedRect.width;
				newRect.width = freeRect.x + freeRect.width - (usedRect.x + usedRect.width);
				newFreeRects.push_back(newRect);
			}
			if (usedRect.y > freeRect.y && usedRect.y < freeRect.y + freeRect.height) {
				Rect newRect = freeRect;
				newRect.height = usedRect.y - newRect.y;
				newFreeRects.push_back(newRect);
			}
			if (usedRect.y + usedRect.height < freeRect.y + freeRect.height) {
				Rect newRect = freeRect;
				newRect.y = usedRect.y + usedRect.height;
				newRect.height = freeRect.y + freeRect.height - (usedRect.y + usedRect.height);
				newFreeRects.push_back(newRect);
			}
			// Supprimer le rectangle libre courant, puisqu'il a été découpé.
			freeRectangles.erase(freeRectangles.begin() + i);
		}
		// Ajouter les nouveaux rectangles libres.
		for (auto &rect : newFreeRects) {
			freeRectangles.push_back(rect);
		}
	}

	// Détermine si deux rectangles se chevauchent.
	bool DoRectanglesIntersect(const Rect& a, const Rect& b) {
		return !(b.x >= a.x + a.width || b.x + b.width <= a.x ||
				 b.y >= a.y + a.height || b.y + b.height <= a.y);
	}

	// Élimine les rectangles libres redondants (contenus dans d'autres).
	void PruneFreeList() {
		for (size_t i = 0; i < freeRectangles.size(); i++) {
			for (size_t j = i + 1; j < freeRectangles.size(); ) {
				if (rectContains(freeRectangles[i], freeRectangles[j])) {
					freeRectangles.erase(freeRectangles.begin() + j);
				} else if (rectContains(freeRectangles[j], freeRectangles[i])) {
					freeRectangles.erase(freeRectangles.begin() + i);
					--i;
					break;
				} else {
					++j;
				}
			}
		}
	}
};

// Structure pour stocker les données d'une texture chargée.
struct TextureData {
	std::string id;
	std::string path;
	int width;
	int height;
	unsigned char* data;
};

// Pour trier les textures par aire décroissante.
bool CompareTextureDataArea(const TextureData& a, const TextureData& b) {
	return (a.width * a.height) > (b.width * b.height);
}

AtlasGenerator::AtlasGenerator(int maxWidth, int maxHeight)
	: m_MaxWidth(maxWidth), m_MaxHeight(maxHeight)
{
}

AtlasGenerator::~AtlasGenerator() {
}

bool AtlasGenerator::generateAtlas(const std::string& jsonConfigFile, Atlas& atlas) {
	// 1. Lecture du fichier JSON de configuration.
	std::ifstream file(jsonConfigFile);
	if (!file.is_open()) {
		std::cerr << "[AtlasGenerator] Impossible d'ouvrir " << jsonConfigFile << std::endl;
		return false;
	}
	json config;
	file >> config;
	file.close();

	// 2. Chargement de toutes les textures listées dans le JSON.
	std::vector<TextureData> textures;
	for (const auto& tex : config["textures"]) {
		TextureData td;
		td.id = tex["id"].get<std::string>();
		td.path = tex["path"].get<std::string>();
		int channels;
		td.data = stbi_load(td.path.c_str(), &td.width, &td.height, &channels, 4); // Forcer le format RGBA.
		if (!td.data) {
			std::cerr << "[AtlasGenerator] Erreur lors du chargement de " << td.path << std::endl;
			for (auto &t : textures)
				stbi_image_free(t.data);
			return false;
		}
		textures.push_back(td);
	}

	// 3. Trier les textures par aire décroissante pour un meilleur packing.
	std::sort(textures.begin(), textures.end(), CompareTextureDataArea);

	// 4. Initialiser le bin packer MaxRects.
	MaxRectsBinPack packer(m_MaxWidth, m_MaxHeight);

	// Pour chaque texture, tenter de l'insérer dans l'atlas.
	std::unordered_map<std::string, AtlasEntry> mapping;
	std::vector<Rect> placedRects;
	for (auto &tex : textures) {
		Rect placement;
		if (!packer.Insert(tex.width, tex.height, placement)) {
			std::cerr << "[AtlasGenerator] Impossible de packer la texture " << tex.id << std::endl;
			for (auto &t : textures)
				stbi_image_free(t.data);
			return false;
		}
		AtlasEntry entry;
		entry.x = placement.x;
		entry.y = placement.y;
		entry.width = tex.width;
		entry.height = tex.height;
		mapping[tex.id] = entry;
		placedRects.push_back(placement);
	}

	// Déterminer la hauteur réellement utilisée dans l'atlas.
	int usedHeight = 0;
	for (const auto& rect : placedRects) {
		int bottom = rect.y + rect.height;
		if (bottom > usedHeight)
			usedHeight = bottom;
	}
	atlas.atlasWidth = m_MaxWidth;
	atlas.atlasHeight = usedHeight;
	atlas.mapping = mapping;

	// 5. Créer la texture OpenGL pour l'atlas.
	glGenTextures(1, &atlas.textureID);
	glBindTexture(GL_TEXTURE_2D, atlas.textureID);
	// Créer une texture vide de la taille (largeur maximale, hauteur utilisée).
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas.atlasWidth, atlas.atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// 6. Copier chaque texture dans l'atlas aux coordonnées assignées.
	for (auto &tex : textures) {
		AtlasEntry entry = atlas.mapping[tex.id];
		glTexSubImage2D(GL_TEXTURE_2D, 0, entry.x, entry.y, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, tex.data);
		stbi_image_free(tex.data);
	}

	// Paramètres de filtrage et d'emballage.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);

	std::cout << "[AtlasGenerator] Atlas généré (" << atlas.atlasWidth << "x" << atlas.atlasHeight << ")." << std::endl;
	return true;
}
