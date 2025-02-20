#include "TextureManager.hpp"
#include <stb_image.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <glad/glad.h>

namespace polymer {
namespace texture {

TextureManager::TextureManager() 
	: atlasTextureID(0), atlasWidth(0), atlasHeight(0) {
}

TextureManager::~TextureManager() {
	freeImages();
	if (atlasTextureID != 0) {
		glDeleteTextures(1, &atlasTextureID);
	}
}

void TextureManager::freeImages() {
	for (auto& img : images) {
		if (img.data) {
			stbi_image_free(img.data);
			img.data = nullptr;
		}
	}
	images.clear();
}

TextureRegion TextureManager::loadTexture(const std::string& filePath) {
	// Si déjà chargé, renvoyer la région
	if (textureRegions.find(filePath) != textureRegions.end()) {
		return textureRegions[filePath];
	}

	ImageData img;
	// Force 4 canaux RGBA
	img.data = stbi_load(filePath.c_str(), &img.width, &img.height, &img.channels, 4);
	if (!img.data) {
		std::cerr << "Erreur de chargement de la texture : " << filePath << std::endl;
		return TextureRegion{ glm::vec2(0.0f), glm::vec2(0.0f) };
	}
	img.channels = 4;
	img.filePath = filePath;
	images.push_back(img);

	// Pour l'instant, on renvoie une région vide ; elle sera mise à jour après la construction de l'atlas.
	TextureRegion region;
	region.uvMin = glm::vec2(0.0f);
	region.uvMax = glm::vec2(0.0f);
	textureRegions[filePath] = region;
	return region;
}

struct Rect {
	int x, y, width, height;
};

bool TextureManager::packImages() {
	// Algorithme naïf de pack : on détermine la taille maximale de chaque image,
	// puis on les dispose en grille.
	int maxWidth = 0;
	int maxHeight = 0;
	for (const auto& img : images) {
		maxWidth = std::max(maxWidth, img.width);
		maxHeight = std::max(maxHeight, img.height);
	}
	if (maxWidth == 0 || maxHeight == 0)
		return false;

	// Calcul du nombre de colonnes et lignes (on arrange en grille carrée)
	int columns = std::ceil(std::sqrt(images.size()));
	int rows = std::ceil((float)images.size() / columns);

	atlasWidth = columns * maxWidth;
	atlasHeight = rows * maxHeight;

	// Créer un buffer pour l'atlas (RGBA)
	std::vector<unsigned char> atlasData(atlasWidth * atlasHeight * 4, 0);

	// Pour chaque image, copier les données dans l'atlas.
	for (size_t i = 0; i < images.size(); ++i) {
		int col = i % columns;
		int row = i / columns;
		int offsetX = col * maxWidth;
		int offsetY = row * maxHeight;

		for (int y = 0; y < images[i].height; ++y) {
			for (int x = 0; x < images[i].width; ++x) {
				int srcIndex = (y * images[i].width + x) * 4;
				int dstX = offsetX + x;
				int dstY = offsetY + y;
				int dstIndex = (dstY * atlasWidth + dstX) * 4;
				atlasData[dstIndex + 0] = images[i].data[srcIndex + 0];
				atlasData[dstIndex + 1] = images[i].data[srcIndex + 1];
				atlasData[dstIndex + 2] = images[i].data[srcIndex + 2];
				atlasData[dstIndex + 3] = images[i].data[srcIndex + 3];
			}
		}

		// Calculer les coordonnées UV normalisées pour cette image
		TextureRegion region;
		region.uvMin = glm::vec2((float)offsetX / atlasWidth, (float)offsetY / atlasHeight);
		region.uvMax = glm::vec2((float)(offsetX + images[i].width) / atlasWidth,
								 (float)(offsetY + images[i].height) / atlasHeight);

		textureRegions[images[i].filePath] = region;
	}

	// Créer la texture OpenGL pour l'atlas
	glGenTextures(1, &atlasTextureID);
	glBindTexture(GL_TEXTURE_2D, atlasTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData.data());

	// Paramètres de la texture (filtrage, wrapping, mipmaps)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool TextureManager::buildAtlas() {
	if (!packImages()) {
		return false;
	}
	freeImages();
	return true;
}

} // namespace texture
} // namespace polymer
