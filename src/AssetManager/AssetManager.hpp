// AssetManager.hpp refactorisé
#pragma once

#include "../Rendering/Shader.hpp"
#include "../Rendering/ShaderProgram.hpp"
#include "../MinePP.hpp"
#include "AssetRegistries.hpp"

class AssetManager
{
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


	void removeTextFromRegistry(const std::string &name) { textRegistry.remove(name); }
	void removeImageFromRegistry(const std::string &name) { imageRegistry.remove(name); }
	void removeTextureFromRegistry(const std::string &name) { textureRegistry.remove(name); }
	void removeCubeMapFromRegistry(const std::string &name) { cubeMapRegistry.remove(name); };
	void removeShaderFromRegistry(const std::string &name) { shaderRegistry.remove(name); }
	void removeShaderProgramFromRegistry(const std::string &name) { shaderProgramRegistry.remove(name); }

	Ref<const std::string> loadText(const std::string &name) { return textRegistry.get(name); };
	Ref<const Image> loadImage(const std::string &name) { return imageRegistry.get(name); };
	Ref<const Texture> loadTexture(const std::string &name) { return textureRegistry.get(name); };
	Ref<const Texture> loadTextureArray(const std::string &name) { return textureArrayRegistry.get(name); };
	Ref<const Texture> loadCubeMap(const std::string &name) { return cubeMapRegistry.get(name); };
	Ref<const Shader> loadShader(const std::string &name) { return shaderRegistry.get(name); };
	Ref<const ShaderProgram> loadShaderProgram(const std::string &name) { return shaderProgramRegistry.get(name); };

	AssetManager(const AssetManager &) = delete;
	AssetManager(AssetManager &) = delete;
	AssetManager(AssetManager &&) noexcept = delete;
	AssetManager &operator=(AssetManager &) = delete;
	AssetManager &operator=(AssetManager &&) noexcept = delete;
};