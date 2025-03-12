// File: src/TextureAtlas.cpp
#include "TextureAtlas.hpp"
#include "../Util/Util.hpp"
#include "AssetManager.hpp"
#include "../Rendering/Image.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

// On utilise nlohmann_json pour le parsing JSON.
// Assurez-vous d'ajouter la librairie (header-only) dans votre projet (par exemple dans external/nlohmann_json).
#include "nlohmann/json.hpp"
using json = nlohmann::json;

TextureAtlas &TextureAtlas::instance()
{
	static TextureAtlas instance;
	return instance;
}

TextureAtlas::TextureAtlas() : tileWidth(16), tileHeight(16), padding(2)
{
	// Charge l'atlas depuis le fichier JSON par défaut
	loadAtlas("assets/textures/textures.json");
}

void TextureAtlas::loadAtlas(const std::string &jsonPath)
{
	auto jsonContent = Util::readBinaryFile(jsonPath);
	if (!jsonContent)
	{
		std::cerr << "Erreur lors du chargement de " << jsonPath << std::endl;
		return;
	}
	json j;
	try
	{
		j = json::parse(*jsonContent);
	}
	catch (json::parse_error &e)
	{
		std::cerr << "Erreur de parsing JSON dans " << jsonPath << ": " << e.what() << std::endl;
		return;
	}

	// Lecture de la configuration d'atlas
	if (j.contains("atlas"))
	{
		auto atlasConfig = j["atlas"];
		tileWidth = atlasConfig.value("tileWidth", 16);
		tileHeight = atlasConfig.value("tileHeight", 16);
		padding = atlasConfig.value("padding", 2);
	}

	// On va collecter toutes les images de tuiles dans un vecteur.
	// Pour éviter de charger plusieurs fois la même image, on utilise un cache (mapping de chemin vers index).
	std::vector<Image> tileImages;
	std::unordered_map<std::string, uint8_t> tileCache; // clé = chemin, valeur = index dans tileImages

	// Fonction lambda pour charger une tuile à partir d'un chemin de fichier
	auto loadTile = [&](const std::string &path) -> uint8_t
	{
		if (tileCache.find(path) != tileCache.end())
		{
			return tileCache[path];
		}
		Ref<const Image> img = AssetManager::instance().loadImage(path);
		if (!img)
		{
			std::cerr << "Échec du chargement de l'image : " << path << std::endl;
			// Crée une image factice (magenta) indiquant une texture manquante
			Image dummy;
			dummy.width = tileWidth;
			dummy.height = tileHeight;
			dummy.data.resize(tileWidth * tileHeight * 4, 255);
			for (size_t i = 0; i < dummy.data.size(); i += 4)
			{
				dummy.data[i] = 255;
				dummy.data[i + 1] = 0;
				dummy.data[i + 2] = 255;
				dummy.data[i + 3] = 255;
			}
			tileImages.push_back(dummy);
			uint8_t index = static_cast<uint8_t>(tileImages.size() - 1);
			tileCache[path] = index;
			return index;
		}
		// Vérification des dimensions (optionnelle)
		if (img->width != static_cast<uint32_t>(tileWidth) || img->height != static_cast<uint32_t>(tileHeight))
		{
			std::cerr << "L'image " << path << " n'a pas les dimensions attendues ("
					  << tileWidth << "x" << tileHeight << ")" << std::endl;
		}
		// Copie de l'image dans le vecteur de tuiles
		Image tile;
		tile.width = img->width;
		tile.height = img->height;
		tile.data = img->data;
		tileImages.push_back(tile);
		uint8_t index = static_cast<uint8_t>(tileImages.size() - 1);
		tileCache[path] = index;
		return index;
	};

	// Parcours de chaque bloc défini dans le JSON
	if (j.contains("blocks") && j["blocks"].is_array())
	{
		for (auto &blockEntry : j["blocks"])
		{
			std::string blockName = blockEntry.value("name", "");
			std::string directory = blockEntry.value("directory", "");
			bool animated = blockEntry.value("animated", false);
			int frames = blockEntry.value("frames", 1);
			float frame_duration = blockEntry.value("frame_duration", 0.0f);

			// Détermine les noms de fichiers pour chaque face.
			// Si la clé "all" est présente, on utilise la même image pour toutes les faces.
			std::array<std::string, 6> faceFiles;
			if (blockEntry["faces"].contains("all"))
			{
				std::string file = blockEntry["faces"]["all"];
				faceFiles.fill(file);
			}
			else
			{
				// Convention : top, bottom, right, left, back, front.
				faceFiles[0] = blockEntry["faces"].value("top", "");
				faceFiles[5] = blockEntry["faces"].value("bottom", "");
				faceFiles[1] = blockEntry["faces"].value("west", "");
				faceFiles[2] = blockEntry["faces"].value("east", "");
				faceFiles[3] = blockEntry["faces"].value("south", "");
				faceFiles[4] = blockEntry["faces"].value("north", "");
			}

			BlockTextureData btd;
			btd.animated = animated;
			btd.frames = frames;
			btd.frame_duration = frame_duration;

			// Pour chaque face, charge la tuile et stocke l’index
			for (int i = 0; i < 6; i++)
			{
				std::string path = directory + "/" + faceFiles[i];
				uint8_t tileIndex = loadTile(path);
				btd.faceIndices[i] = tileIndex;
			}

			// Convertir le nom de bloc en BlockData::BlockType.
			// Ici, on suppose que le nom (en minuscules) correspond à l’enum.
			BlockData::BlockType type = BlockData::BlockType::air;
			if (blockName == "bedrock")
				type = BlockData::BlockType::bedrock;
			else if (blockName == "planks")
				type = BlockData::BlockType::planks;
			else if (blockName == "grass")
				type = BlockData::BlockType::grass;
			else if (blockName == "dirt")
				type = BlockData::BlockType::dirt;
			else if (blockName == "sand")
				type = BlockData::BlockType::sand;
			else if (blockName == "stone")
				type = BlockData::BlockType::stone;
			else if (blockName == "cobblestone")
				type = BlockData::BlockType::cobblestone;
			else if (blockName == "glass")
				type = BlockData::BlockType::glass;
			else if (blockName == "oak_wood")
				type = BlockData::BlockType::oak_wood;
			else if (blockName == "oak_leaves")
				type = BlockData::BlockType::oak_leaves;
			else if (blockName == "water")
				type = BlockData::BlockType::water;
			else if (blockName == "lava")
				type = BlockData::BlockType::lava;
			else if (blockName == "iron")
				type = BlockData::BlockType::iron;
			else if (blockName == "diamond")
				type = BlockData::BlockType::diamond;
			else if (blockName == "gold")
				type = BlockData::BlockType::gold;
			else if (blockName == "obsidian")
				type = BlockData::BlockType::obsidian;
			else if (blockName == "sponge")
				type = BlockData::BlockType::sponge;

			mapping[type] = btd;
		}
	}

	// Crée le texture array à partir du vecteur de tuiles.
	atlasTexture = Texture::loadTexture2DArrayFromImages(tileImages, tileWidth, tileHeight);
}

BlockTextureData TextureAtlas::getBlockTextureData(BlockData::BlockType type) const
{
	if (mapping.find(type) != mapping.end())
	{
		return mapping.at(type);
	}
	// Si introuvable, renvoie une configuration par défaut (toutes les faces = 0)
	BlockTextureData defaultMapping;
	defaultMapping.animated = false;
	defaultMapping.frames = 1;
	defaultMapping.frame_duration = 0;
	defaultMapping.faceIndices.fill(0);
	return defaultMapping;
}
