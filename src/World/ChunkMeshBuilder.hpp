/**
 * @file ChunkMeshBuilder.hpp
 * @brief Thread-safe mesh building for chunks
 * 
 * @details Separates mesh generation logic from the Chunk class to enable
 *          multithreaded mesh building. The builder can work on chunk data
 *          without modifying the chunk itself.
 */

#pragma once

#include "../Common.hpp"
#include "../Rendering/BlockVertex.hpp"
#include "BlockTypes.hpp"
#include "LODLevel.hpp"
#include <vector>

class Chunk;
class World;
class Assets;

/**
 * @brief Data structure containing the result of mesh building
 * 
 * @details This structure can be passed between threads safely
 */
struct ChunkMeshData {
    std::vector<BlockVertex> solidVertices;
    std::vector<BlockVertex> semiTransparentVertices;
    int32_t solidVertexCount = 0;
    int32_t semiTransparentVertexCount = 0;
    
    /**
     * @brief Clear all data
     */
    void clear() {
        solidVertices.clear();
        semiTransparentVertices.clear();
        solidVertexCount = 0;
        semiTransparentVertexCount = 0;
    }
    
    /**
     * @brief Reserve capacity for vertices
     */
    void reserve(size_t capacity) {
        solidVertices.reserve(capacity);
        semiTransparentVertices.reserve(capacity);
    }
};

/**
 * @class ChunkMeshBuilder
 * @brief Builds mesh data for chunks in a thread-safe manner
 * 
 * @details This class extracts the mesh building logic from Chunk to allow
 *          for parallel mesh generation. It reads chunk data but doesn't
 *          modify the chunk state.
 */
class ChunkMeshBuilder {
private:
    /**
     * @brief Checks if a block at the given position is not air
     * 
     * @param pos Position to check (can be outside chunk bounds)
     * @param chunk The chunk containing the block
     * @param world World reference for checking adjacent chunks
     * @return true if the block exists and is not air, false otherwise
     */
    static bool hasNonAirAt(const glm::ivec3& pos, const Chunk& chunk, const World& world);
    
    /**
     * @brief Calculates ambient occlusion level for a vertex
     * 
     * @param blockPos Position of the block
     * @param vertOffset Offset of the vertex from the block position
     * @param chunk The chunk containing the block
     * @param world World reference for checking adjacent chunks
     * @param useAmbientOcclusion Whether to calculate AO or return default
     * @return Occlusion level from 0 (fully occluded) to 3 (no occlusion)
     */
    static uint8_t calculateOcclusionLevel(const glm::ivec3& blockPos,
                                          const glm::ivec3& vertOffset,
                                          const Chunk& chunk,
                                          const World& world,
                                          bool useAmbientOcclusion);

public:
    /**
     * @brief Build mesh data for a chunk
     * 
     * @details This method is thread-safe and doesn't modify the chunk.
     *          It generates vertex data that can be uploaded to the GPU later.
     * 
     * @param chunk The chunk to build mesh for
     * @param world The world containing the chunk
     * @param assets Assets for texture lookups
     * @param useAmbientOcclusion Whether to calculate ambient occlusion
     * @param outMeshData Output mesh data
     * 
     * @note This method can be called from any thread
     */
    static void buildMesh(const Chunk& chunk, 
                         const World& world,
                         const Assets& assets,
                         bool useAmbientOcclusion,
                         ChunkMeshData& outMeshData,
                         LODLevel lod = LODLevel::Full);
    
    /**
     * @brief Estimate the number of vertices a chunk might need
     * 
     * @param chunk The chunk to estimate for
     * @return Estimated vertex count
     */
    static int32_t estimateVertexCount(const Chunk& chunk);
};