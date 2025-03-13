/**
 * @class CubeMapRegistry
 * @brief Registre spécialisé dans le chargement des textures cubemap.
 *
 * @details Hérite d'AssetRegistry et implémente loadAsset() pour charger une texture cubemap via Texture::loadCubeMapTexture().
 */

#pragma once

#include "AssetRegistry.hpp"

class CubeMapRegistry : public AssetRegistry<Texture>
{
	Ref<const Texture> loadAsset(const std::string &name) override { return Texture::loadCubeMapTexture(name); };
};