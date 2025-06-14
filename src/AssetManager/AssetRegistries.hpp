// Nouveau fichier : consolidation de tous les registres d'assets
#pragma once

#include "AssetRegistry.hpp"
#include "../Rendering/Shader.hpp"
#include "../Rendering/ShaderProgram.hpp"
#include "../Rendering/Textures.hpp"
#include "../Utils/Utils.hpp"

class AssetManager;

class TextRegistry : public AssetRegistry<std::string>
{
	Ref<const std::string> loadAsset(const std::string &name) override { return Util::readBinaryFile(name); }
};

class ImageRegistry : public AssetRegistry<Image>
{
	Ref<const Image> loadAsset(const std::string &name) override
	{
		Ref<Image> image = std::make_shared<Image>();
		uint32_t error = lodepng::decode(image->data, image->width, image->height, name, LCT_RGBA);
		if (error != 0)
		{
			std::cerr << "Failed to read the image file: " << name << ", message: " << lodepng_error_text(error) << std::endl;
			return nullptr;
		}
		return image;
	}
};

class TextureRegistry : public AssetRegistry<Texture>
{
	AssetManager& assetManager;

public:
	TextureRegistry(AssetManager& assetManager) : assetManager(assetManager) {}
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadTexture2D(name, assetManager); }
};

class TextureArrayRegistry : public AssetRegistry<Texture>
{
	AssetManager& assetManager;

public:
	TextureArrayRegistry(AssetManager& assetManager) : assetManager(assetManager) {}
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadTexture2DArray(name, assetManager); }
};

class CubeMapRegistry : public AssetRegistry<Texture>
{
	AssetManager& assetManager;

public:
	CubeMapRegistry(AssetManager& assetManager) : assetManager(assetManager) {}
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadCubeMapTexture(name, assetManager); }
};

class ShaderRegistry : public AssetRegistry<Shader>
{
	AssetManager& assetManager;

public:
	ShaderRegistry(AssetManager& assetManager) : assetManager(assetManager) {}
	Ref<const Shader> loadAsset(const std::string &name) override { return std::make_shared<Shader>(name, assetManager); }
};

class ShaderProgramRegistry : public AssetRegistry<ShaderProgram>
{
	AssetManager& assetManager;

public:
	ShaderProgramRegistry(AssetManager& assetManager) : assetManager(assetManager) {}
	Ref<const ShaderProgram> loadAsset(const std::string &name) override { return std::make_shared<ShaderProgram>(name, assetManager); }
};