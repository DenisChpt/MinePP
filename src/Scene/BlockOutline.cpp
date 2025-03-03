#include "BlockOutline.hpp"

#include "../AssetManager/AssetManager.hpp"

BlockOutline::BlockOutline(Ref<const CubeMesh> blockMesh)
    : outlinedBlockShader(AssetManager::instance().loadShaderProgram("assets/shaders/outline")),
      blockMesh(std::move(blockMesh)) {}

void BlockOutline::render(const glm::mat4& transform) const {
  outlinedBlockShader->setMat4("MVP", transform);
  outlinedBlockShader->bind();
  blockMesh->render();
}
