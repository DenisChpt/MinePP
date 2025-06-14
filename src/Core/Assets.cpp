#include "Assets.hpp"

#include "../Rendering/Shaders.hpp"
#include "../Rendering/Textures.hpp"
#include "../Utils/Utils.hpp"

#include <nlohmann/json.hpp>

#include <lodepng.h>

#include <fstream>
#include <iostream>

Assets::Assets() {
	// Load texture atlas on initialization
	loadTextureAtlas();
}

Assets::~Assets() {
	// Destructor - smart pointers handle cleanup
}

template <typename T>
Ref<const T> Assets::getFromCache(Cache<T>& cache,
								  const std::string& name,
								  std::function<Ref<const T>()> loader) {
	auto it = cache.find(name);
	if (it != cache.end()) {
		if (auto asset = it->second.lock()) {
			return asset;
		}
	}

	auto asset = loader();
	if (asset) {
		cache[name] = asset;
	}
	return asset;
}

Ref<const std::string> Assets::loadText(const std::string& path) {
	return getFromCache<std::string>(texts, path, [&path]() { return Util::readBinaryFile(path); });
}

Ref<const Image> Assets::loadImage(const std::string& path) {
	return getFromCache<Image>(images, path, [&path]() {
		auto image = std::make_shared<Image>();
		uint32_t error = lodepng::decode(image->data, image->width, image->height, path, LCT_RGBA);
		if (error != 0) {
			std::cerr << "Failed to read the image file: " << path
					  << ", message: " << lodepng_error_text(error) << std::endl;
			return Ref<Image>(nullptr);
		}
		return image;
	});
}

Ref<const Texture> Assets::loadTexture(const std::string& path) {
	return getFromCache<Texture>(
		textures, path, [this, &path]() { return Texture::loadTexture2D(path, *this); });
}

Ref<const Texture> Assets::loadTexture2DArray(const std::string& path) {
	return getFromCache<Texture>(
		textureArrays, path, [this, &path]() { return Texture::loadTexture2DArray(path, *this); });
}

Ref<const Texture> Assets::loadCubeMap(const std::string& path) {
	return getFromCache<Texture>(
		cubeMaps, path, [this, &path]() { return Texture::loadCubeMapTexture(path, *this); });
}

Ref<const Shader> Assets::loadShader(const std::string& path) {
	return getFromCache<Shader>(
		shaders, path, [this, &path]() { return std::make_shared<Shader>(path, *this); });
}

Ref<const ShaderProgram> Assets::loadShaderProgram(const std::string& path) {
	return getFromCache<ShaderProgram>(
		programs, path, [this, &path]() { return std::make_shared<ShaderProgram>(path, *this); });
}

void Assets::loadTextureAtlas(const std::string& jsonPath) {
	atlas.loadAtlas(jsonPath, *this);
}

// TextureAtlas implementation
void Assets::TextureAtlas::loadAtlas(const std::string& jsonPath, Assets& assets) {
	TRACE_FUNCTION();

	// Load JSON configuration
	std::ifstream file(jsonPath);
	if (!file.is_open()) {
		std::cerr << "Could not open texture atlas configuration: " << jsonPath << std::endl;
		return;
	}

	nlohmann::json j;
	file >> j;

	// Read atlas configuration
	tileWidth = j["atlas"]["tileWidth"];
	tileHeight = j["atlas"]["tileHeight"];
	padding = j["atlas"].value("padding", 0);

	std::vector<std::vector<uint8_t>> textureData;

	// Load all texture files from blocks array
	for (const auto& block : j["blocks"]) {
		std::string blockName = block["name"];

		// Convert string to BlockType
		BlockData::BlockType blockType = BlockData::BlockType::air;
		if (blockName == "bedrock")
			blockType = BlockData::BlockType::bedrock;
		else if (blockName == "planks")
			blockType = BlockData::BlockType::planks;
		else if (blockName == "grass")
			blockType = BlockData::BlockType::grass;
		else if (blockName == "dirt")
			blockType = BlockData::BlockType::dirt;
		else if (blockName == "sand")
			blockType = BlockData::BlockType::sand;
		else if (blockName == "stone")
			blockType = BlockData::BlockType::stone;
		else if (blockName == "cobblestone")
			blockType = BlockData::BlockType::cobblestone;
		else if (blockName == "glass")
			blockType = BlockData::BlockType::glass;
		else if (blockName == "oak_wood")
			blockType = BlockData::BlockType::oak_wood;
		else if (blockName == "oak_leaves")
			blockType = BlockData::BlockType::oak_leaves;
		else if (blockName == "water")
			blockType = BlockData::BlockType::water;
		else if (blockName == "lava")
			blockType = BlockData::BlockType::lava;
		else if (blockName == "iron")
			blockType = BlockData::BlockType::iron;
		else if (blockName == "diamond")
			blockType = BlockData::BlockType::diamond;
		else if (blockName == "gold")
			blockType = BlockData::BlockType::gold;
		else if (blockName == "obsidian")
			blockType = BlockData::BlockType::obsidian;
		else if (blockName == "sponge")
			blockType = BlockData::BlockType::sponge;
		BlockTextureData blockData;

		// Check if animated
		blockData.animated = block.value("animated", false);
		if (blockData.animated) {
			blockData.frames = block.value("frames", 1);
			blockData.frame_duration = block.value("frame_duration", 0.0f);
		} else {
			blockData.frames = 1;
			blockData.frame_duration = 0;
		}

		// Load face textures
		std::string directory = block["directory"];
		auto faces = block["faces"];

		const std::array<std::string, 6> faceNames = {
			"top", "east", "west", "north", "south", "bottom"};
		for (int i = 0; i < 6; i++) {
			std::string texturePath;

			// Check if this face is specified or use "all"
			if (faces.contains(faceNames[i])) {
				texturePath = faces[faceNames[i]];
			} else if (faces.contains("all")) {
				texturePath = faces["all"];
			} else {
				std::cerr << "No texture found for face " << faceNames[i] << " of block "
						  << blockName << std::endl;
				continue;
			}

			// Load texture image
			std::string fullPath = directory + "/" + texturePath;
			auto image = assets.loadImage(fullPath);
			if (!image) {
				std::cerr << "Failed to load texture: " << fullPath << std::endl;
				continue;
			}

			// For animated textures, expect vertical tile layout
			// For static textures, use the entire image
			if (blockData.animated) {
				int tilesPerCol = image->height / tileHeight;
				for (int frame = 0; frame < std::min(blockData.frames, tilesPerCol); frame++) {
					std::vector<uint8_t> tileData(tileWidth * tileHeight * 4);

					// Copy tile data from vertical strip
					for (int y = 0; y < tileHeight; y++) {
						for (int x = 0; x < tileWidth; x++) {
							int srcX = x;
							int srcY = frame * tileHeight + y;
							int srcIdx = (srcY * image->width + srcX) * 4;
							int dstIdx = (y * tileWidth + x) * 4;

							if (srcIdx + 3 < image->data.size()) {
								for (int c = 0; c < 4; c++) {
									tileData[dstIdx + c] = image->data[srcIdx + c];
								}
							}
						}
					}

					blockData.faceIndices[i] = textureData.size();
					textureData.push_back(std::move(tileData));
				}
			} else {
				// Static texture - use entire image
				std::vector<uint8_t> tileData(tileWidth * tileHeight * 4);

				// Resize or crop image to fit tileWidth x tileHeight
				for (int y = 0; y < tileHeight; y++) {
					for (int x = 0; x < tileWidth; x++) {
						int srcX = x * image->width / tileWidth;
						int srcY = y * image->height / tileHeight;
						int srcIdx = (srcY * image->width + srcX) * 4;
						int dstIdx = (y * tileWidth + x) * 4;

						if (srcIdx + 3 < image->data.size()) {
							for (int c = 0; c < 4; c++) {
								tileData[dstIdx + c] = image->data[srcIdx + c];
							}
						} else {
							// Fill with white if out of bounds
							for (int c = 0; c < 4; c++) {
								tileData[dstIdx + c] = 255;
							}
						}
					}
				}

				blockData.faceIndices[i] = textureData.size();
				textureData.push_back(std::move(tileData));
			}
		}

		mapping[blockType] = blockData;
	}

	// Create texture array
	atlasTexture =
		std::make_shared<Texture>(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, true);
	atlasTexture->bind();

	glTexImage3D(GL_TEXTURE_2D_ARRAY,
				 0,
				 GL_RGBA8,
				 tileWidth,
				 tileHeight,
				 textureData.size(),
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 nullptr);

	for (size_t i = 0; i < textureData.size(); i++) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,
						0,
						0,
						i,
						tileWidth,
						tileHeight,
						1,
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						textureData[i].data());
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	atlasTexture->unbind();

	std::cout << "Texture atlas created successfully with " << textureData.size()
			  << " textures for " << mapping.size() << " block types." << std::endl;
}

BlockTextureData Assets::TextureAtlas::getBlockTextureData(BlockData::BlockType type) const {
	auto it = mapping.find(type);
	if (it != mapping.end()) {
		return it->second;
	}

	// Return default texture data if not found
	BlockTextureData defaultData;
	defaultData.animated = false;
	defaultData.frames = 1;
	defaultData.frame_duration = 0;
	std::fill(defaultData.faceIndices.begin(), defaultData.faceIndices.end(), 0);
	return defaultData;
}