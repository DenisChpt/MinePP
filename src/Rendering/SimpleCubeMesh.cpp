#include "SimpleCubeMesh.hpp"
#include <glad/glad.h>

SimpleCubeMesh::SimpleCubeMesh() {
    // Simple cube vertices (position only)
    const std::vector<float> vertices = {
        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        
        // Back face
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        
        // Top face
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        
        // Bottom face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        
        // Right face
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
        
        // Left face
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f
    };
    
    // Create vertex buffer
    vertexBuffer = std::make_shared<VertexBuffer>();
    vertexBuffer->bind();
    
    // Convert float array to vec3 array for the buffer
    std::vector<glm::vec3> vertexData;
    vertexData.reserve(VERTEX_COUNT);
    for (size_t i = 0; i < vertices.size(); i += 3) {
        vertexData.emplace_back(vertices[i], vertices[i+1], vertices[i+2]);
    }
    vertexBuffer->bufferStaticData(vertexData);
    
    // Create vertex array with vertex attributes
    std::vector<VertexAttribute> attributes = {
        VertexAttribute(3, VertexAttribute::Float, 0)  // position
    };
    vertexArray = std::make_shared<VertexArray>(vertexData, attributes);
    
    vertexArray->unbind();
}

void SimpleCubeMesh::render() const {
    vertexArray->bind();
    glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);
    vertexArray->unbind();
}