/**
 * @class TextureAtlas
 * @brief Gestionnaire d'atlas de textures pour le rendu des blocs.
 *
 * @details La classe TextureAtlas charge la configuration des textures depuis un fichier JSON (textures.json),
 *          découpe les images en tuiles et crée un texture array unique. Elle fournit également la configuration (indices de faces)
 *          pour chaque type de bloc.
 *
 * @note Implémentée en singleton.
 */

#pragma once

#include "../MinePP.hpp"
#include "../Rendering/Texture.hpp"
#include "../World/BlockData.hpp"
#include <array>
#include <unordered_map>
#include <string>

// Structure qui contiendra pour un bloc la correspondance des 6 faces (ordre : top, east, west, north, south, bottom)
// ainsi que des informations pour les textures animées.
struct BlockTextureData
{
	std::array<uint8_t, 6> faceIndices; // Chaque valeur correspond à la couche dans le texture array.
	bool animated;
	int frames;
	float frame_duration;
};

class AssetManager;

class TextureAtlas
{
public:
	TextureAtlas(AssetManager& assetManager);

	// Charge l'atlas à partir du fichier JSON donné (par défaut "assets/textures/textures.json")
	void loadAtlas(const std::string &jsonPath);

	// Renvoie le texture array construit
	Ref<const Texture> getAtlasTexture() const { return atlasTexture; }

	// Renvoie la configuration de textures pour un type de bloc
	BlockTextureData getBlockTextureData(BlockData::BlockType type) const;

private:
	AssetManager& assetManager;

	Ref<const Texture> atlasTexture;
	int tileWidth;
	int tileHeight;
	int padding;

	// Mapping de BlockData::BlockType vers la configuration de texture (pour les 6 faces et animation)
	std::unordered_map<BlockData::BlockType, BlockTextureData> mapping;
};
