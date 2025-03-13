/**
 * @class AssetManager
 * @brief Gestionnaire central des ressources (assets) du projet.
 *
 * @details La classe AssetManager permet de charger et de mettre en cache les assets tels que les shaders, textures,
 *          images et fichiers texte. Elle fait appel à plusieurs registres spécialisés (AssetRegistry) pour éviter les rechargements
 *          inutiles et garantir une utilisation efficace de la mémoire.
 *
 * @note Des méthodes de suppression et de récupération sont fournies pour chaque type d'asset.
 */


#pragma once

#include "../Rendering/Shader.hpp"
#include "../Rendering/ShaderProgram.hpp"
#include "../MinePP.hpp"
#include "AssetRegistry.hpp"
#include "CubeMapRegistry.hpp"
#include "ImageRegistry.hpp"
#include "ShaderProgramRegistry.hpp"
#include "ShaderRegistry.hpp"
#include "TextRegistry.hpp"
#include "TextureArrayRegistry.hpp"
#include "TextureRegistry.hpp"

class AssetManager
{
	static AssetManager *instancePtr;

	TextRegistry textRegistry;
	ImageRegistry imageRegistry;
	TextureRegistry textureRegistry;
	TextureArrayRegistry textureArrayRegistry;
	CubeMapRegistry cubeMapRegistry;
	ShaderRegistry shaderRegistry;
	ShaderProgramRegistry shaderProgramRegistry;

public:
	AssetManager();
	~AssetManager();

	static AssetManager &instance() { return *instancePtr; };

	void removeTextFromRegistry(const std::string &name) { textRegistry.remove(name); }
	void removeImageFromRegistry(const std::string &name) { imageRegistry.remove(name); }
	void removeTextureFromRegistry(const std::string &name) { textureRegistry.remove(name); }
	void removeCubeMapFromRegistry(const std::string &name) { cubeMapRegistry.remove(name); };
	void removeShaderFromRegistry(const std::string &name) { shaderRegistry.remove(name); }
	void removeShaderProgramFromRegistry(const std::string &name) { shaderProgramRegistry.remove(name); }

	Ref<const std::string> loadText(const std::string &name) { return textRegistry.get(name); };
	Ref<const Image> loadImage(const std::string &name) { return imageRegistry.get(name); };
	Ref<const Texture> loadTexture(const std::string &name) { return textureRegistry.get(name); };

	/// the expected input format: width;height;
	Ref<const Texture> loadTextureArray(const std::string &name) { return textureArrayRegistry.get(name); };

	/// the expected input format: right;left;top;bottom;front;back
	Ref<const Texture> loadCubeMap(const std::string &name) { return cubeMapRegistry.get(name); };
	Ref<const Shader> loadShader(const std::string &name) { return shaderRegistry.get(name); };
	Ref<const ShaderProgram> loadShaderProgram(const std::string &name) { return shaderProgramRegistry.get(name); };

	AssetManager(const AssetManager &) = delete;
	AssetManager(AssetManager &) = delete;
	AssetManager(AssetManager &&) noexcept = delete;
	AssetManager &operator=(AssetManager &) = delete;
	AssetManager &operator=(AssetManager &&) noexcept = delete;
};
