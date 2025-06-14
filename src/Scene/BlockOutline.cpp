#include "BlockOutline.hpp"

#include "../Core/Assets.hpp"

BlockOutline::BlockOutline(Ref<const CubeMesh> blockMesh, Assets& assets)
	: outlinedBlockShader(assets.loadShaderProgram("assets/shaders/outline")),
	  blockMesh(std::move(blockMesh)) {}

void BlockOutline::render(const glm::mat4 &transform) const
{
	outlinedBlockShader->setMat4("MVP", transform);
	outlinedBlockShader->bind();
	blockMesh->render();
}
