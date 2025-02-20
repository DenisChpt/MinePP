#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace polymer {
namespace texture {

// Structure représentant la région d'une texture dans l'atlas (en coordonnées UV normalisées)
struct TextureRegion {
	glm::vec2 uvMin;
	glm::vec2 uvMax;
};

// La classe TextureManager gère le chargement d'images et la création d'un atlas OpenGL
class TextureManager {
public:
	TextureManager();
	~TextureManager();

	// Charge une texture depuis un fichier et la stocke dans l'atlas.
	// Retourne la région associée à cette texture.
	TextureRegion loadTexture(const std::string& filePath);

	// Une fois toutes les textures chargées, construisez l'atlas.
	// Retourne true en cas de succès.
	bool buildAtlas();

	// Accès à l'ID OpenGL de l'atlas.
	GLuint getAtlasTextureID() const { return atlasTextureID; }

private:
	struct ImageData {
		int width;
		int height;
		int channels;
		unsigned char* data;
		std::string filePath;
	};

	// Liste des images chargées
	std::vector<ImageData> images;

	// Mapping de chemin de fichier vers la région dans l'atlas
	std::unordered_map<std::string, TextureRegion> textureRegions;

	// ID OpenGL de la texture atlas
	GLuint atlasTextureID;

	// Dimensions de l'atlas
	int atlasWidth;
	int atlasHeight;

	// Libère les images chargées
	void freeImages();

	// Fonction naïve de pack d'images dans l'atlas
	bool packImages();
};

} // namespace texture
} // namespace polymer

#endif // TEXTURE_MANAGER_HPP
