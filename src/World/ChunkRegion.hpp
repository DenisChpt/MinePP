/**
 * @file ChunkRegion.hpp
 * @brief Hierarchical structure for optimized frustum culling
 * 
 * @details Groups chunks into regions for efficient culling.
 *          If a region is outside the frustum, all its chunks can be skipped.
 */

#pragma once

#include "../Common.hpp"
#include "../Math/Math.hpp"
#include "Chunk.hpp"
#include <Frustum.h>

/**
 * @class ChunkRegion
 * @brief Groups multiple chunks for hierarchical frustum culling
 * 
 * @details Each region contains a grid of chunks and maintains a bounding box
 *          for the entire region. This allows for quick rejection of multiple
 *          chunks at once during frustum culling.
 */
class ChunkRegion {
public:
    /**
     * @brief Size of a region in chunks (e.g., 4x4 chunks per region)
     */
    static constexpr int32_t RegionSize = 4;
    
    /**
     * @brief Size of a region in world units (not used anymore, kept for compatibility)
     * @deprecated Use RegionSize * Chunk::HorizontalSize directly
     */
    static constexpr int32_t RegionWorldSize = RegionSize * Chunk::HorizontalSize;

private:
    glm::ivec2 regionPosition;  // Position in region coordinates
    AABB boundingBox;
    std::vector<Ref<Chunk>> chunks;
    bool dirty = true;

    /**
     * @brief Updates the bounding box based on contained chunks
     */
    void updateBoundingBox();

public:
    /**
     * @brief Constructs a region at the given region coordinates
     * 
     * @param regionPos Position in region space (not world space)
     */
    explicit ChunkRegion(const glm::ivec2& regionPos);

    /**
     * @brief Adds a chunk to this region
     * 
     * @param chunk The chunk to add
     * @note The chunk must be within this region's bounds
     */
    void addChunk(const Ref<Chunk>& chunk);

    /**
     * @brief Removes a chunk from this region
     * 
     * @param chunkWorldPos World position of the chunk to remove
     * @return true if chunk was found and removed
     */
    bool removeChunk(const glm::ivec2& chunkWorldPos);

    /**
     * @brief Checks if the region is visible in the frustum
     * 
     * @param frustum The view frustum
     * @return true if any part of the region might be visible
     */
    [[nodiscard]] bool isVisible(const Frustum& frustum) const {
        return frustum.IsBoxVisible(boundingBox.minPoint, boundingBox.maxPoint);
    }

    /**
     * @brief Gets all chunks in this region
     */
    [[nodiscard]] const std::vector<Ref<Chunk>>& getChunks() const { return chunks; }

    /**
     * @brief Checks if the region has any chunks
     */
    [[nodiscard]] bool isEmpty() const { return chunks.empty(); }

    /**
     * @brief Gets the region position
     */
    [[nodiscard]] glm::ivec2 getPosition() const { return regionPosition; }

    /**
     * @brief Converts world chunk position to region position
     * 
     * @param chunkWorldPos Chunk position in world coordinates
     * @return Region position
     */
    static glm::ivec2 chunkToRegionPos(const glm::ivec2& chunkWorldPos) {
        // First convert world coordinates to chunk indices
        int32_t chunkX = chunkWorldPos.x / Chunk::HorizontalSize;
        int32_t chunkZ = chunkWorldPos.y / Chunk::HorizontalSize;
        
        // Then convert chunk indices to region indices
        return glm::ivec2(
            std::floor(static_cast<float>(chunkX) / RegionSize),
            std::floor(static_cast<float>(chunkZ) / RegionSize)
        );
    }

    /**
     * @brief Checks if a chunk position belongs to this region
     * 
     * @param chunkWorldPos Chunk position in world coordinates
     * @return true if the chunk belongs to this region
     */
    [[nodiscard]] bool containsChunk(const glm::ivec2& chunkWorldPos) const {
        glm::ivec2 regionOfChunk = chunkToRegionPos(chunkWorldPos);
        return regionOfChunk == regionPosition;
    }
};