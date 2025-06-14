#pragma once

#include "../Rendering/ShaderProgram.hpp"
#include "CubeMesh.hpp"

class AssetManager;

class BlockOutline
{
	Ref<const ShaderProgram> outlinedBlockShader;
	Ref<const CubeMesh> blockMesh;

public:
	BlockOutline(Ref<const CubeMesh> blockMesh, AssetManager& assetManager);

	void render(const glm::mat4 &transform) const;
};