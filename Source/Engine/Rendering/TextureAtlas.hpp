#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <vector>


class TextureAtlas {
public:
	/**
	 * @brief Construit un atlas à partir d'un fichier image.
	 * @param filePath Chemin vers l'image de l'atlas (ex. "Assets/Textures/Atlas/BlocksAtlas.png").
	 * @param gridSize Nombre de textures par ligne/colonne (ex. 16 pour un atlas 16x16).
	 */
	TextureAtlas(const std::string& filePath, unsigned int gridSize);
	~TextureAtlas();

	/**
	 * @brief Charge l'atlas en mémoire et crée la texture OpenGL.
	 * @return true si le chargement a réussi.
	 */
	bool load();

	/**
	 * @brief Renvoie l'ID de la texture OpenGL.
	 */
	unsigned int getTextureID() const;

	/**
	 * @brief Calcule les coordonnées UV pour une sous-image de l'atlas.
	 * @param index L'index de la texture dans la grille (de 0 à gridSize*gridSize - 1).
	 * @return Un tableau de 4 glm::vec2 représentant (u,v) en ordre : 
	 *         { bottom-left, bottom-right, top-right, top-left }.
	 */
	std::vector<glm::vec2> getUVCoordinates(unsigned int index) const;

private:
	std::string m_FilePath;
	unsigned int m_GridSize;
	unsigned int m_TextureID;
	int m_AtlasWidth, m_AtlasHeight;
	bool m_Loaded;

	// Méthode interne pour charger l'image (exemple avec stb_image)
	bool loadImage();
};
