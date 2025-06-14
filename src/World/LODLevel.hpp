#pragma once

#include <cstdint>

/**
 * @brief Level of Detail for chunk rendering
 */
enum class LODLevel : uint8_t {
    Full = 0,     // Full detail - all blocks
    Half = 1,     // Half detail - simplified mesh
    Quarter = 2,  // Quarter detail - heavily simplified mesh
    Count = 3     // Number of LOD levels
};

/**
 * @brief Helper class for LOD calculations
 */
class LODSelector {
public:
    // Distance thresholds for LOD transitions (in chunks)
    static constexpr float FULL_LOD_DISTANCE = 4.0f;
    static constexpr float HALF_LOD_DISTANCE = 8.0f;
    
    /**
     * @brief Select appropriate LOD level based on distance
     * @param distanceInChunks Distance from camera in chunk units
     * @return Selected LOD level
     */
    static LODLevel selectLOD(float distanceInChunks) {
        if (distanceInChunks <= FULL_LOD_DISTANCE) {
            return LODLevel::Full;
        } else if (distanceInChunks <= HALF_LOD_DISTANCE) {
            return LODLevel::Half;
        } else {
            return LODLevel::Quarter;
        }
    }
    
    /**
     * @brief Get the block skip factor for a LOD level
     * @param lod The LOD level
     * @return How many blocks to skip (1 = no skip, 2 = every other, etc.)
     */
    static int getBlockSkipFactor(LODLevel lod) {
        switch (lod) {
            case LODLevel::Full:
                return 1;
            case LODLevel::Half:
                return 2;
            case LODLevel::Quarter:
                return 4;
            default:
                return 1;
        }
    }
};