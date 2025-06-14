#pragma once

#include "../Rendering/Shaders.hpp"
#include "../Rendering/Mesh.hpp"

class Assets;

class BlockOutline
{
	Ref<const ShaderProgram> outlinedBlockShader;
	Ref<const CubeMesh> blockMesh;

public:
	BlockOutline(Ref<const CubeMesh> blockMesh, Assets& assets);

	void render(const glm::mat4 &transform) const;
};