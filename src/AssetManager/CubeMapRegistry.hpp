#pragma once

#include "AssetRegistry.hpp"

class CubeMapRegistry : public AssetRegistry<Texture>
{
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadCubeMapTexture(name); };
};