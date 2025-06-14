#pragma once
#include "../Common.hpp"
#include "../World/BlockTypes.hpp"

#include <array>
#include <functional>
#include <string>
#include <unordered_map>

// Forward declarations
class Shader;
class ShaderProgram;
class Texture;
struct Image;

// Structure qui contiendra pour un bloc la correspondance des 6 faces (ordre : top, east, west,
// north, south, bottom) ainsi que des informations pour les textures animées.
struct BlockTextureData {
	std::array<uint8_t, 6>
		faceIndices;  // Chaque valeur correspond à la couche dans le texture array.
	bool animated;
	int frames;
	float frame_duration;
};

class Assets {
	// Un seul cache générique
	template <typename T>
	using Cache = std::unordered_map<std::string, std::weak_ptr<const T>>;

	Cache<std::string> texts;
	Cache<Image> images;
	Cache<Texture> textures;
	Cache<Texture> textureArrays;
	Cache<Texture> cubeMaps;
	Cache<Shader> shaders;
	Cache<ShaderProgram> programs;

	// TextureAtlas intégré
	struct TextureAtlas {
		Ref<const Texture> atlasTexture;
		int tileWidth;
		int tileHeight;
		int padding;

		// Mapping de BlockData::BlockType vers la configuration de texture (pour les 6 faces et
		// animation)
		std::unordered_map<BlockData::BlockType, BlockTextureData> mapping;

		void loadAtlas(const std::string& jsonPath, Assets& assets);
		BlockTextureData getBlockTextureData(BlockData::BlockType type) const;
	} atlas;

	// Helper methods for cache management
	template <typename T>
	Ref<const T> getFromCache(Cache<T>& cache,
							  const std::string& name,
							  std::function<Ref<const T>()> loader);

public:
	Assets();
	~Assets();

	// Text and image loading
	Ref<const std::string> loadText(const std::string& path);
	Ref<const Image> loadImage(const std::string& path);

	// Texture loading with different types
	Ref<const Texture> loadTexture(const std::string& path);
	Ref<const Texture> loadTexture2DArray(const std::string& path);
	Ref<const Texture> loadCubeMap(const std::string& path);

	// Shader loading
	Ref<const Shader> loadShader(const std::string& path);
	Ref<const ShaderProgram> loadShaderProgram(const std::string& path);

	// Atlas access
	const TextureAtlas& getAtlas() const { return atlas; }
	Ref<const Texture> getAtlasTexture() const { return atlas.atlasTexture; }
	BlockTextureData getBlockTextureData(BlockData::BlockType type) const {
		return atlas.getBlockTextureData(type);
	}

	// Load texture atlas
	void loadTextureAtlas(const std::string& jsonPath = "assets/textures/textures.json");
};

// Temporary alias for smooth transition
using AssetManager = Assets;