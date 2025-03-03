#pragma once

#include "../Rendering/VertexArray.hpp"

class CubeMesh {
  Ref<VertexArray> cubeVertexArray;

public:
  CubeMesh();
  void render() const;
};
