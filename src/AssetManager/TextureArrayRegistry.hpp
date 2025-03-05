#pragma once

#include "../Rendering/Texture.hpp"
#include "AssetRegistry.hpp"

class TextureArrayRegistry : public AssetRegistry<Texture>
{
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadTexture2DArray(name); }
};
