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
#include "nlohmann/json.hpp"
using json = nlohmann::json;

TextureAtlas::TextureAtlas(AssetManager& assetManager) 
	: assetManager(assetManager), tileWidth(16), tileHeight(16), padding(2)
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

	// On va collecter toutes les images (tuiles ou frames) dans un seul vecteur.
	// Pour éviter de recharger le même chemin plusieurs fois, on utilise un cache
	// (mapping: chemin -> indexDeLaPremiereFrame).
	std::vector<Image> tileImages;
	std::unordered_map<std::string, uint8_t> tileCache; // clé = path, valeur = index de la première frame

	// Lambda pour charger un spritesheet (ou une seule tuile) et renvoyer l'index de la première frame.
	auto loadTileOrFrames = [&](const std::string &path, bool animated, int framesCount) -> uint8_t
	{
		// Si on a déjà chargé ce path, on récupère l'index de la première frame.
		if (tileCache.find(path) != tileCache.end())
		{
			return tileCache[path];
		}

		// On tente de charger l'image
		Ref<const Image> img = assetManager.loadImage(path);
		if (!img)
		{
			std::cerr << "Échec du chargement de l'image : " << path << std::endl;

			// On crée (framesCount) fois une image factice (rose flashy) si c'est animé,
			// sinon 1 seule si framesCount==1
			uint8_t firstIndex = static_cast<uint8_t>(tileImages.size());
			int total = (animated && framesCount > 1) ? framesCount : 1;
			for (int f = 0; f < total; f++)
			{
				Image dummy;
				dummy.width = tileWidth;
				dummy.height = tileHeight;
				dummy.data.resize(tileWidth * tileHeight * 4, 255);
				for (size_t i = 0; i < dummy.data.size(); i += 4)
				{
					dummy.data[i + 0] = 255; // R
					dummy.data[i + 1] = 0;	 // G
					dummy.data[i + 2] = 255; // B
					dummy.data[i + 3] = 255; // A
				}
				tileImages.push_back(dummy);
			}
			tileCache[path] = firstIndex;
			return firstIndex;
		}

		// Si l'image est animée (plusieurs frames), on s'attend à un spritesheet
		// vertical de dimension (tileWidth x (tileHeight*framesCount)).
		if (animated && framesCount > 1)
		{
			// Vérifier la taille
			uint32_t expectedW = tileWidth;
			uint32_t expectedH = tileHeight * framesCount;
			if (img->width != expectedW || img->height != expectedH)
			{
				std::cerr << "Spritesheet " << path
						  << " : dimensions inattendues ("
						  << img->width << "x" << img->height
						  << "), attendues = ("
						  << expectedW << "x" << expectedH
						  << ") pour " << framesCount << " frames.\n";
			}
			uint8_t firstIndex = static_cast<uint8_t>(tileImages.size());

			// On découpe chaque frame
			for (int f = 0; f < framesCount; f++)
			{
				Image frame = img->subImage(
					{0, (uint32_t)(f * tileHeight)}, // offset (x=0, y=f*tileHeight)
					{(uint32_t)tileWidth, (uint32_t)tileHeight});
				tileImages.push_back(frame);
			}
			tileCache[path] = firstIndex;
			return firstIndex;
		}
		else
		{
			// Cas non-animé (ou framesCount=1), on attend une image 16x16 (ou tileWidth x tileHeight)
			if (img->width != (uint32_t)tileWidth || img->height != (uint32_t)tileHeight)
			{
				std::cerr << "L'image " << path << " n'a pas les dimensions attendues ("
						  << tileWidth << "x" << tileHeight << "), mais ("
						  << img->width << "x" << img->height << ")!\n";
			}
			// On stocke directement la tuile
			Image tile;
			tile.width = img->width;
			tile.height = img->height;
			tile.data = img->data;

			uint8_t index = static_cast<uint8_t>(tileImages.size());
			tileImages.push_back(tile);
			tileCache[path] = index;
			return index;
		}
	};

	// On parcourt chaque bloc défini dans le JSON
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
			// Si "all" est présent, même image pour toutes les faces.
			std::array<std::string, 6> faceFiles;
			if (blockEntry["faces"].contains("all"))
			{
				std::string file = blockEntry["faces"]["all"];
				faceFiles.fill(file);
			}
			else
			{
				// top, bottom, right, left, back, front (ordre interne).
				faceFiles[0] = blockEntry["faces"].value("top", "");
				faceFiles[5] = blockEntry["faces"].value("bottom", "");
				faceFiles[1] = blockEntry["faces"].value("west", "");
				faceFiles[2] = blockEntry["faces"].value("east", "");
				faceFiles[3] = blockEntry["faces"].value("south", "");
				faceFiles[4] = blockEntry["faces"].value("north", "");
			}

			// On crée la structure pour ce bloc
			BlockTextureData btd;
			btd.animated = animated;
			btd.frames = frames;
			btd.frame_duration = frame_duration;

			// Pour chaque face, on charge la tuile (ou le spritesheet)
			for (int i = 0; i < 6; i++)
			{
				if (faceFiles[i].empty())
				{
					// Si vide => on met un dummy
					btd.faceIndices[i] = 0; // ou n'importe
					continue;
				}
				std::string path = directory + "/" + faceFiles[i];
				uint8_t firstFrameIndex = loadTileOrFrames(path, animated, frames);
				btd.faceIndices[i] = firstFrameIndex;
			}

			// Convertir le nom en BlockData::BlockType (simplifié)
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
			// Sinon => air par défaut

			mapping[type] = btd;
		}
	}

	// Enfin, on crée le texture array unique à partir du vecteur de tuiles
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
