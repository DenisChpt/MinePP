/**
 * @file ChunkMeshTask.hpp
 * @brief Task structure for async mesh building
 * 
 * @details Defines the task structure that can be passed to worker threads
 *          for parallel mesh generation
 */

#pragma once

#include "../Common.hpp"
#include "ChunkMeshBuilder.hpp"
#include <atomic>
#include <memory>

/**
 * @brief Status of a mesh building task
 */
enum class MeshTaskStatus {
    Pending,    // Task created but not started
    Building,   // Task is being processed
    Complete,   // Task completed successfully
    Failed      // Task failed
};

/**
 * @class ChunkMeshTask
 * @brief Represents a mesh building task that can be processed asynchronously
 * 
 * @details This class encapsulates all data needed to build a chunk mesh
 *          on a worker thread. It includes the input data (chunk position)
 *          and output data (mesh vertices).
 */
class ChunkMeshTask {
private:
    glm::ivec2 chunkPosition;
    std::atomic<MeshTaskStatus> status{MeshTaskStatus::Pending};
    ChunkMeshData meshData;
    
    // Optional error message if task failed
    std::string errorMessage;
    
public:
    /**
     * @brief Construct a new mesh task
     * 
     * @param position The chunk position
     */
    explicit ChunkMeshTask(const glm::ivec2& position) 
        : chunkPosition(position) {}
    
    /**
     * @brief Get the chunk position
     */
    [[nodiscard]] glm::ivec2 getChunkPosition() const { return chunkPosition; }
    
    /**
     * @brief Get the current status
     */
    [[nodiscard]] MeshTaskStatus getStatus() const { return status.load(); }
    
    /**
     * @brief Set the task status
     */
    void setStatus(MeshTaskStatus newStatus) { status.store(newStatus); }
    
    /**
     * @brief Get the mesh data (only valid if status is Complete)
     */
    [[nodiscard]] const ChunkMeshData& getMeshData() const { return meshData; }
    [[nodiscard]] ChunkMeshData& getMeshData() { return meshData; }
    
    /**
     * @brief Set error message (for Failed status)
     */
    void setError(const std::string& error) {
        errorMessage = error;
        setStatus(MeshTaskStatus::Failed);
    }
    
    /**
     * @brief Get error message
     */
    [[nodiscard]] const std::string& getError() const { return errorMessage; }
    
    /**
     * @brief Check if task is ready to be processed
     */
    [[nodiscard]] bool isReady() const { 
        return status.load() == MeshTaskStatus::Pending; 
    }
    
    /**
     * @brief Check if task is complete
     */
    [[nodiscard]] bool isComplete() const { 
        return status.load() == MeshTaskStatus::Complete; 
    }
};

using ChunkMeshTaskPtr = std::shared_ptr<ChunkMeshTask>;