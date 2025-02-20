#include "TextureManager.hpp"

#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <vector>

// Inclusion de stb_image (vérifiez que le chemin correspond à votre configuration)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureManager::TextureManager()
	: atlasTextureID(0), atlasWidth(0), atlasHeight(0), textureWidth(0), textureHeight(0)
{
}

TextureManager::~TextureManager() {
	clearAtlas();
}

void TextureManager::clearAtlas() {
	if (atlasTextureID != 0) {
		glDeleteTextures(1, &atlasTextureID);
		atlasTextureID = 0;
	}
	textureRegions.clear();
}

bool TextureManager::loadTextureAtlas(const std::vector<std::pair<std::string, std::string>>& textures) {
	// Inverse verticalement les images pour correspondre à la convention OpenGL.
	stbi_set_flip_vertically_on_load(true);

	std::vector<unsigned char*> imageData;
	int w, h, ch;

	// Chargement de chaque image via stb_image.
	for (const auto& tex : textures) {
		unsigned char* data = stbi_load(tex.second.c_str(), &w, &h, &ch, 4); // Forçage en RGBA
		if (!data) {
			std::cerr << "Erreur : échec du chargement de la texture : " << tex.second << std::endl;
			for (auto d : imageData)
				stbi_image_free(d);
			return false;
		}
		if (imageData.empty()) {
			// La première image définit la taille des textures.
			textureWidth = w;
			textureHeight = h;
		} else {
			// Vérifie que toutes les textures ont les mêmes dimensions.
			if (w != textureWidth || h != textureHeight) {
				std::cerr << "Erreur : dimensions différentes pour la texture : " << tex.second << std::endl;
				for (auto d : imageData)
					stbi_image_free(d);
				stbi_image_free(data);
				return false;
			}
		}
		imageData.push_back(data);
	}

	// Détermine la taille de la grille de l'atlas.
	int count = textures.size();
	int gridCols = std::ceil(std::sqrt(count));
	int gridRows = std::ceil(static_cast<float>(count) / gridCols);
	atlasWidth = gridCols * textureWidth;
	atlasHeight = gridRows * textureHeight;

	// Alloue un buffer pour l'atlas (4 canaux par pixel).
	std::vector<unsigned char> atlasData(atlasWidth * atlasHeight * 4, 0);

	// Copie de chaque image dans le buffer de l'atlas.
	for (int index = 0; index < count; ++index) {
		int row = index / gridCols;
		int col = index % gridCols;
		int xOffset = col * textureWidth;
		int yOffset = row * textureHeight;
		unsigned char* src = imageData[index];

		for (int y = 0; y < textureHeight; ++y) {
			for (int x = 0; x < textureWidth; ++x) {
				int atlasX = xOffset + x;
				int atlasY = yOffset + y;
				int atlasIndex = (atlasY * atlasWidth + atlasX) * 4;
				int srcIndex = (y * textureWidth + x) * 4;
				atlasData[atlasIndex + 0] = src[srcIndex + 0];
				atlasData[atlasIndex + 1] = src[srcIndex + 1];
				atlasData[atlasIndex + 2] = src[srcIndex + 2];
				atlasData[atlasIndex + 3] = src[srcIndex + 3];
			}
		}

		// Calcule les coordonnées UV pour cette texture dans l'atlas.
		TextureRegion region;
		region.u  = static_cast<float>(xOffset) / atlasWidth;
		region.v  = static_cast<float>(yOffset) / atlasHeight;
		region.u2 = static_cast<float>(xOffset + textureWidth) / atlasWidth;
		region.v2 = static_cast<float>(yOffset + textureHeight) / atlasHeight;
		textureRegions[textures[index].first] = region;

		// Libère la mémoire de l'image individuelle.
		stbi_image_free(src);
	}

	// Création de la texture OpenGL pour l'atlas.
	glGenTextures(1, &atlasTextureID);
	glBindTexture(GL_TEXTURE_2D, atlasTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	// Paramétrage du filtrage et de l'interpolation.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Trilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Gestion de l'anisotropie si disponible.
	if (GLAD_GL_EXT_texture_filter_anisotropic) {
		GLfloat maxAnisotropy = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

unsigned int TextureManager::getAtlasTextureID() const {
	return atlasTextureID;
}

void TextureManager::bindAtlasTexture(GLenum textureUnit) const {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, atlasTextureID);
}

bool TextureManager::getTextureRegion(const std::string& textureName, TextureRegion& region) const {
	auto it = textureRegions.find(textureName);
	if (it != textureRegions.end()) {
		region = it->second;
		return true;
	}
	return false;
}
