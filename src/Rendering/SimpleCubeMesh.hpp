#pragma once

#include "../Common.hpp"
#include "Buffers.hpp"
#include <vector>

/**
 * @class SimpleCubeMesh
 * @brief A simple cube mesh with only position data for particle rendering
 */
class SimpleCubeMesh {
private:
    Ref<VertexArray> vertexArray;
    Ref<VertexBuffer> vertexBuffer;
    static constexpr GLsizei VERTEX_COUNT = 36; // 6 faces * 2 triangles * 3 vertices
    
public:
    SimpleCubeMesh();
    
    // Get vertex array for binding
    const VertexArray& getVertexArray() const { return *vertexArray; }
    
    // Get vertex buffer for instanced rendering setup
    VertexBuffer& getVertexBuffer() { return *vertexBuffer; }
    
    // Get number of vertices
    GLsizei getVertexCount() const { return VERTEX_COUNT; }
    
    // Render the cube (non-instanced)
    void render() const;
};