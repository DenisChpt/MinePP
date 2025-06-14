#pragma once

#include "../Common.hpp"
#include "Buffers.hpp"
#include "SimpleCubeMesh.hpp"
#include "Shaders.hpp"
#include <vector>

/**
 * @brief Data for a single particle instance
 */
struct ParticleInstanceData {
    glm::mat4 transform;
    glm::vec4 color;
};

/**
 * @class InstancedParticleRenderer
 * @brief Handles instanced rendering of particles for better performance
 * 
 * @details This class manages the instance buffer and provides methods
 *          for efficient rendering of many particles with a single draw call
 */
class InstancedParticleRenderer {
private:
    // Vertex buffer for instance data
    VertexBuffer instanceVBO;
    
    // Vertex array object configured for instancing
    std::unique_ptr<VertexArray> vao;
    GLuint customVAOId = 0;
    
    // Cube mesh for particles
    SimpleCubeMesh cubeMesh;
    
    // Maximum number of instances
    static constexpr size_t MAX_INSTANCES = 10000;
    
public:
    /**
     * @brief Constructor
     */
    InstancedParticleRenderer();
    
    /**
     * @brief Destructor
     */
    ~InstancedParticleRenderer();
    
    /**
     * @brief Update instance data and render all particles
     * 
     * @param instances Vector of instance data (transform + color)
     * @param shader Shader program to use (must support instancing)
     */
    void render(const std::vector<ParticleInstanceData>& instances, 
                const Ref<const ShaderProgram>& shader);
    
    /**
     * @brief Get the maximum number of instances supported
     */
    size_t getMaxInstances() const { return MAX_INSTANCES; }
    
private:
    /**
     * @brief Setup the vertex array object for instanced rendering
     */
    void setupInstancedVAO();
};