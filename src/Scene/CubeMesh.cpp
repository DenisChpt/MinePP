#include "CubeMesh.hpp"

#include "../Rendering/BlockMesh.hpp"
#include "../Rendering/BlockVertex.hpp"

CubeMesh::CubeMesh() {
  std::vector<BlockVertex> vertices;

  vertices.resize(6 * 6);
  int32_t vertexCount = 0;
  for (const auto& face: BlockMesh::vertices) {
    for (const auto& vertex: face) {
      vertices.at(vertexCount) = vertex;
      vertexCount++;
    }
  }
  cubeVertexArray = std::make_shared<VertexArray>(vertices, BlockVertex::vertexAttributes());
}

void CubeMesh::render() const {
  cubeVertexArray->renderVertexStream();
}