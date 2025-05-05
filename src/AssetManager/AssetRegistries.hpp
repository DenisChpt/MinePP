// Nouveau fichier : consolidation de tous les registres d'assets
#pragma once

#include "AssetRegistry.hpp"
#include "../Rendering/Shader.hpp"
#include "../Rendering/ShaderProgram.hpp"
#include "../Rendering/Image.hpp"
#include "../Rendering/Texture.hpp"
#include "../Util/Util.hpp"

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
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadTexture2D(name); }
};

class TextureArrayRegistry : public AssetRegistry<Texture>
{
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadTexture2DArray(name); }
};

class CubeMapRegistry : public AssetRegistry<Texture>
{
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadCubeMapTexture(name); }
};

class ShaderRegistry : public AssetRegistry<Shader>
{
	Ref<const Shader> loadAsset(const std::string &name) override { return std::make_shared<Shader>(name); }
};

class ShaderProgramRegistry : public AssetRegistry<ShaderProgram>
{
	Ref<const ShaderProgram> loadAsset(const std::string &name) override { return std::make_shared<ShaderProgram>(name); }
};