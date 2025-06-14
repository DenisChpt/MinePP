#include "AssetManager.hpp"

AssetManager::AssetManager()
	: textRegistry(),
	  imageRegistry(),
	  textureRegistry(*this),
	  textureArrayRegistry(*this),
	  cubeMapRegistry(*this),
	  shaderRegistry(*this),
	  shaderProgramRegistry(*this)
{
};

AssetManager::~AssetManager()
{
};
