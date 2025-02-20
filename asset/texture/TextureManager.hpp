#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <glad/glad.h>

// Structure représentant la région UV d'une texture dans l'atlas.
struct TextureRegion {
	float u;  // Coordonnée u de départ
	float v;  // Coordonnée v de départ
	float u2; // Coordonnée u de fin
	float v2; // Coordonnée v de fin
};

class TextureManager {
public:
	TextureManager();
	~TextureManager();

	/**
	 * Charge et assemble une texture atlas à partir d'une liste de textures.
	 * @param textures : vecteur de paires (nom de la texture, chemin du fichier image)
	 * @return true si le chargement et l'assemblage ont réussi.
	 */
	bool loadTextureAtlas(const std::vector<std::pair<std::string, std::string>>& textures);

	/**
	 * Retourne l'ID OpenGL de la texture atlas.
	 */
	unsigned int getAtlasTextureID() const;

	/**
	 * Lie la texture atlas sur une unité donnée.
	 * @param textureUnit : unité de texture OpenGL (ex. GL_TEXTURE0)
	 */
	void bindAtlasTexture(GLenum textureUnit = GL_TEXTURE0) const;

	/**
	 * Récupère la région UV associée au nom d'une texture.
	 * @param textureName : nom de la texture recherchée.
	 * @param region : (out) région UV dans l'atlas.
	 * @return true si trouvée, false sinon.
	 */
	bool getTextureRegion(const std::string& textureName, TextureRegion& region) const;

private:
	unsigned int atlasTextureID;
	int atlasWidth;
	int atlasHeight;
	int textureWidth;
	int textureHeight;
	std::unordered_map<std::string, TextureRegion> textureRegions;

	/// Libère les ressources éventuellement allouées pour l'atlas.
	void clearAtlas();
};

#endif // TEXTURE_MANAGER_HPP
