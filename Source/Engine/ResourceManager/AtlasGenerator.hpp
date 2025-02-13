#pragma once
#include <string>
#include <unordered_map>

/// Coordonnées d'une texture dans l'atlas.
struct AtlasEntry {
	int x;       // Position x en pixels dans l'atlas.
	int y;       // Position y en pixels dans l'atlas.
	int width;   // Largeur de la texture.
	int height;  // Hauteur de la texture.
};

/// Structure contenant l'ID OpenGL de l'atlas et le mapping de textures.
struct Atlas {
	unsigned int textureID;  // L'ID OpenGL de l'atlas généré.
	int atlasWidth;          // Largeur totale de l'atlas.
	int atlasHeight;         // Hauteur totale (utilisée) de l'atlas.
	/// Mapping des identifiants (issus du JSON) vers leurs coordonnées dans l'atlas.
	std::unordered_map<std::string, AtlasEntry> mapping;
};

class AtlasGenerator {
public:
	/**
	 * @brief Construit l'AtlasGenerator avec la taille maximale autorisée.
	 * @param maxWidth Largeur maximale de l'atlas.
	 * @param maxHeight Hauteur maximale de l'atlas.
	 */
	AtlasGenerator(int maxWidth, int maxHeight);
	~AtlasGenerator();

	/**
	 * @brief Génère l'atlas en lisant le fichier JSON de configuration.
	 * @param jsonConfigFile Chemin vers le fichier JSON.
	 * @param atlas L'objet Atlas qui sera rempli.
	 * @return true si la génération a réussi.
	 */
	bool generateAtlas(const std::string& jsonConfigFile, Atlas& atlas);

private:
	int m_MaxWidth;
	int m_MaxHeight;

	// Vous pouvez ajouter ici d'éventuelles fonctions internes auxiliaires.
};
