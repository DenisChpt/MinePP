#include "InstancedParticleRenderer.hpp"
#include <glad/glad.h>

InstancedParticleRenderer::InstancedParticleRenderer() {
    setupInstancedVAO();
}

InstancedParticleRenderer::~InstancedParticleRenderer() {
    if (customVAOId != 0) {
        glDeleteVertexArrays(1, &customVAOId);
    }
}

void InstancedParticleRenderer::setupInstancedVAO() {
    // We'll manually create a VAO for instanced rendering
    GLuint vaoId;
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    
    // Bind the cube mesh's VBO for vertex data
    const auto& cubeVAO = cubeMesh.getVertexArray();
    auto& cubeVBO = cubeMesh.getVertexBuffer();
    
    // Setup vertex attributes from cube mesh
    cubeVBO.bind();
    // Position attribute at location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Setup instance buffer
    instanceVBO.bind();
    
    // Reserve space for maximum instances
    std::vector<ParticleInstanceData> dummyData(MAX_INSTANCES);
    instanceVBO.bufferDynamicData(dummyData);
    
    // Setup instance attributes
    // Transform matrix (4x4) uses locations 1-4
    size_t stride = sizeof(ParticleInstanceData);
    for (int i = 0; i < 4; ++i) {
        glVertexAttribPointer(
            1 + i,                                          // location
            4,                                              // size (vec4)
            GL_FLOAT,                                       // type
            GL_FALSE,                                       // normalize
            stride,                                         // stride
            (void*)(offsetof(ParticleInstanceData, transform) + i * sizeof(glm::vec4))  // offset
        );
        glEnableVertexAttribArray(1 + i);
        glVertexAttribDivisor(1 + i, 1); // This attribute advances per instance
    }
    
    // Color attribute at location 5
    glVertexAttribPointer(
        5,                                    // location
        4,                                    // size (vec4)
        GL_FLOAT,                             // type
        GL_FALSE,                             // normalize
        stride,                               // stride
        (void*)offsetof(ParticleInstanceData, color) // offset
    );
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1); // This attribute advances per instance
    
    glBindVertexArray(0);
    
    // Create a wrapper VertexArray to manage the VAO
    vao = std::make_unique<VertexArray>();
    // Hack: directly set the VAO id (this is not clean but works for now)
    // We need to store the VAO id somehow
    customVAOId = vaoId;
}

void InstancedParticleRenderer::render(const std::vector<ParticleInstanceData>& instances,
                                     const Ref<const ShaderProgram>& shader) {
    if (instances.empty()) {
        return;
    }
    
    // Limit instances to maximum
    size_t instanceCount = std::min(instances.size(), MAX_INSTANCES);
    
    // Update instance buffer with new data
    instanceVBO.bind();
    // Use bufferDynamicSubData to update the buffer
    std::vector<ParticleInstanceData> tempData(instances.begin(), instances.begin() + instanceCount);
    instanceVBO.bufferDynamicSubData(tempData, instanceCount, 0, 0);
    
    // Bind shader and custom VAO
    shader->bind();
    glBindVertexArray(customVAOId);
    
    // Draw instanced
    glDrawArraysInstanced(GL_TRIANGLES, 0, cubeMesh.getVertexCount(), static_cast<GLsizei>(instanceCount));
    
    glBindVertexArray(0);
}