#include "TextureModule.hpp"
#include <stb_image.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

using json = nlohmann::json;

namespace asset {
namespace texture {

	////////////////////////////////////////////////////////////////////////////////
	//                            TextureAtlas Implementation                     //
	////////////////////////////////////////////////////////////////////////////////

	TextureAtlas::TextureAtlas()
		: atlasTextureID(0), atlasWidth(0), atlasHeight(0),
		  textureWidth(0), textureHeight(0)
	{
	}

	TextureAtlas::~TextureAtlas() {
		clearAtlas();
	}

	void TextureAtlas::clearAtlas() {
		if (atlasTextureID != 0) {
			glDeleteTextures(1, &atlasTextureID);
			atlasTextureID = 0;
		}
		textureRegions.clear();
	}

	// Fonction utilitaire pour charger une image depuis un fichier.
	static unsigned char* loadImage(const std::string& filePath, int& width, int& height, int& channels) {
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 4);
		if (!data) {
			std::cerr << "Error: Failed to load image: " << filePath << std::endl;
		}
		return data;
	}

	// Packe les images dans un atlas.
	bool TextureAtlas::packTextures(const std::vector<std::pair<std::string, std::string>>& textures) {
		if (textures.empty()) return false;

		int w, h, ch;
		// Charger la première image pour déterminer la taille commune.
		unsigned char* firstData = loadImage(textures[0].second, w, h, ch);
		if (!firstData) return false;
		textureWidth = w;
		textureHeight = h;
		stbi_image_free(firstData);

		// Charger toutes les images et vérifier leur cohérence.
		std::vector<unsigned char*> images;
		for (const auto& tex : textures) {
			int iw, ih, ich;
			unsigned char* data = loadImage(tex.second, iw, ih, ich);
			if (!data) {
				for (auto d : images)
					stbi_image_free(d);
				return false;
			}
			if (iw != textureWidth || ih != textureHeight) {
				std::cerr << "Error: Inconsistent dimensions in texture: " << tex.second << std::endl;
				for (auto d : images)
					stbi_image_free(d);
				stbi_image_free(data);
				return false;
			}
			images.push_back(data);
		}

		int count = textures.size();
		int gridCols = std::ceil(std::sqrt(count));
		int gridRows = std::ceil(static_cast<float>(count) / gridCols);
		atlasWidth = gridCols * textureWidth;
		atlasHeight = gridRows * textureHeight;

		std::vector<unsigned char> atlasData(atlasWidth * atlasHeight * 4, 0);

		// Copier chaque image dans l’atlas et calculer la région UV.
		for (size_t i = 0; i < images.size(); ++i) {
			int col = i % gridCols;
			int row = i / gridCols;
			int xOffset = col * textureWidth;
			int yOffset = row * textureHeight;
			unsigned char* src = images[i];
			for (int y = 0; y < textureHeight; ++y) {
				for (int x = 0; x < textureWidth; ++x) {
					int srcIndex = (y * textureWidth + x) * 4;
					int dstX = xOffset + x;
					int dstY = yOffset + y;
					int dstIndex = (dstY * atlasWidth + dstX) * 4;
					atlasData[dstIndex + 0] = src[srcIndex + 0];
					atlasData[dstIndex + 1] = src[srcIndex + 1];
					atlasData[dstIndex + 2] = src[srcIndex + 2];
					atlasData[dstIndex + 3] = src[srcIndex + 3];
				}
			}
			TextureRegion region;
			region.uvMin = glm::vec2(static_cast<float>(xOffset) / atlasWidth,
									 static_cast<float>(yOffset) / atlasHeight);
			region.uvMax = glm::vec2(static_cast<float>(xOffset + textureWidth) / atlasWidth,
									 static_cast<float>(yOffset + textureHeight) / atlasHeight);
			textureRegions[textures[i].first] = region;
		}

		for (auto d : images)
			stbi_image_free(d);

		// Création de la texture OpenGL.
		glGenTextures(1, &atlasTextureID);
		glBindTexture(GL_TEXTURE_2D, atlasTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	bool TextureAtlas::loadAtlas(const std::vector<TextureDefinition>& textureDefs) {
		std::vector<std::pair<std::string, std::string>> textures;
		for (const auto& def : textureDefs) {
			textures.push_back({ def.name, def.file });
		}
		return packTextures(textures);
	}

	bool TextureAtlas::getTextureRegion(const std::string& name, TextureRegion& region) const {
		auto it = textureRegions.find(name);
		if (it != textureRegions.end()) {
			region = it->second;
			return true;
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////
	//                           TextureManager Implementation                    //
	////////////////////////////////////////////////////////////////////////////////

	TextureManager::TextureManager() {}

	TextureManager::~TextureManager() {}

	// 1. Charge la configuration JSON
	bool TextureManager::loadTextureConfig(const std::string& configFilePath) {
		std::ifstream file(configFilePath);
		if (!file.is_open()) {
			std::cerr << "Error: Cannot open texture config file: " << configFilePath << std::endl;
			return false;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		json j;
		try {
			j = json::parse(buffer.str());
		} catch (json::parse_error &e) {
			std::cerr << "JSON parse error in texture config: " << e.what() << std::endl;
			return false;
		}
		if (!j.is_object()) {
			std::cerr << "Error: Texture config must be a JSON object." << std::endl;
			return false;
		}
		// Parcours de chaque entrée et création d'une TextureDefinition.
		for (auto it = j.begin(); it != j.end(); ++it) {
			TextureDefinition def;
			def.name = it.key();
			if (it.value().contains("file") && it.value()["file"].is_string()) {
				def.file = it.value()["file"].get<std::string>();
			} else {
				std::cerr << "Texture " << def.name << " missing required 'file' field." << std::endl;
				continue;
			}
			if (it.value().contains("modifiers") && it.value()["modifiers"].is_string()) {
				def.modifiers = it.value()["modifiers"].get<std::string>();
			}
			if (it.value().contains("animation") && it.value()["animation"].is_object()) {
				json anim = it.value()["animation"];
				def.animation.frametime = anim.value("frametime", 1);
				if (anim.contains("frames") && anim["frames"].is_array()) {
					for (const auto& frame : anim["frames"]) {
						if (frame.is_number_integer())
							def.animation.frames.push_back(frame.get<int>());
					}
				}
			}
			textureDefinitions[def.name] = def;
		}
		return true;
	}

	// 2. Initialise l'atlas en utilisant les définitions chargées.
	bool TextureManager::initializeAtlas() {
		std::vector<TextureDefinition> defs;
		for (const auto& pair : textureDefinitions)
			defs.push_back(pair.second);
		return atlas.loadAtlas(defs);
	}

	// 3. Récupère la définition d'une texture par nom.
	bool TextureManager::getTextureDefinition(const std::string& name, TextureDefinition& def) const {
		auto it = textureDefinitions.find(name);
		if (it != textureDefinitions.end()) {
			def = it->second;
			return true;
		}
		return false;
	}

	// 4. Retourne l'ID de l'atlas OpenGL.
	GLuint TextureManager::getAtlasTextureID() const {
		return atlas.getAtlasTextureID();
	}

	bool TextureManager::getTextureRegion(const std::string& name, TextureRegion& region) const {
		return atlas.getTextureRegion(name, region);
	}

} // namespace texture
} // namespace asset
