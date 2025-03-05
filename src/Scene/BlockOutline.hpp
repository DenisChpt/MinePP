#pragma once

#include "../Rendering/ShaderProgram.hpp"
#include "CubeMesh.hpp"

class BlockOutline
{
	Ref<const ShaderProgram> outlinedBlockShader;
	Ref<const CubeMesh> blockMesh;

public:
	explicit BlockOutline(Ref<const CubeMesh> blockMesh);

	void render(const glm::mat4 &transform) const;
};