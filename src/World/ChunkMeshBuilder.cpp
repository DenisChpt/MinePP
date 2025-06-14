#include "ChunkMeshBuilder.hpp"
#include "Chunk.hpp"
#include "World.hpp"
#include "../Core/Assets.hpp"

bool ChunkMeshBuilder::hasNonAirAt(const glm::ivec3& pos, const Chunk& chunk, const World& world) {
    const BlockData* block = chunk.getBlockAtOptimized(pos, world);
    return block != nullptr && block->blockClass != BlockData::BlockClass::air;
}

uint8_t ChunkMeshBuilder::calculateOcclusionLevel(const glm::ivec3& blockPos,
                                                  const glm::ivec3& vertOffset,
                                                  const Chunk& chunk,
                                                  const World& world,
                                                  bool useAmbientOcclusion) {
    if (!useAmbientOcclusion) {
        return 3; // No occlusion
    }
    
    glm::ivec3 direction = glm::sign(glm::vec3(vertOffset) - glm::vec3(.5));
    
    uint8_t side1 = hasNonAirAt(blockPos + direction * glm::ivec3(1, 1, 0), chunk, world) ? 1 : 0;
    uint8_t side2 = hasNonAirAt(blockPos + direction * glm::ivec3(0, 1, 1), chunk, world) ? 1 : 0;
    if (side1 && side2) {
        return 0;
    }
    
    uint8_t corner = hasNonAirAt(blockPos + direction * glm::ivec3(1, 1, 1), chunk, world) ? 1 : 0;
    return 3 - (side1 + side2 + corner);
}

void ChunkMeshBuilder::buildMesh(const Chunk& chunk, 
                                const World& world,
                                const Assets& assets,
                                bool useAmbientOcclusion,
                                ChunkMeshData& outMeshData) {
    TRACE_FUNCTION();
    
    // Clear output data
    outMeshData.clear();
    
    // Reserve estimated capacity
    int32_t estimatedVertices = estimateVertexCount(chunk);
    outMeshData.reserve(estimatedVertices);
    
    // Direction offsets for face checking
    const std::array<glm::ivec3, 6> offsetsToCheck = {{
        {1, 0, 0},
        {-1, 0, 0},
        {0, 1, 0},
        {0, -1, 0},
        {0, 0, 1},
        {0, 0, -1},
    }};
    
    // Iterate through all blocks in the chunk
    for (int32_t x = Chunk::HorizontalSize - 1; x >= 0; --x) {
        for (int32_t y = Chunk::VerticalSize - 1; y >= 0; --y) {
            for (int32_t z = Chunk::HorizontalSize - 1; z >= 0; --z) {
                glm::ivec3 blockPos = {x, y, z};
                const BlockData* blockData = chunk.getBlockAt(blockPos);
                if (!blockData || blockData->blockClass == BlockData::BlockClass::air) {
                    continue;
                }
                
                const auto& [type, blockClass] = *blockData;
                
                // Check each face
                for (const glm::ivec3& offset : offsetsToCheck) {
                    const BlockData* neighborBlock = chunk.getBlockAtOptimized(blockPos + offset, world);
                    if (neighborBlock == nullptr) {
                        continue;
                    }
                    
                    bool isSameClass = neighborBlock->blockClass == blockClass;
                    bool isTransparentNextToOpaque =
                        neighborBlock->blockClass == BlockData::BlockClass::solid &&
                        blockClass == BlockData::BlockClass::transparent;
                    if (isSameClass || isTransparentNextToOpaque) {
                        continue;
                    }
                    
                    // Generate vertices for this face
                    for (const auto& vertex : BlockMesh::getVerticesFromDirection(offset)) {
                        BlockVertex vert = vertex;
                        vert.offset(x, y, z);
                        vert.setType(offset, type, assets);
                        
                        uint8_t occlusionLevel = 3;
                        if (useAmbientOcclusion) {
                            if (offset.y == -1) {
                                occlusionLevel = 0;
                            } else {
                                occlusionLevel = calculateOcclusionLevel(
                                    blockPos, vert.getPosition() - blockPos, chunk, world, useAmbientOcclusion);
                            }
                        }
                        vert.setOcclusionLevel(occlusionLevel);
                        
                        // Add to appropriate vertex list
                        if (blockClass == BlockData::BlockClass::semiTransparent ||
                            blockClass == BlockData::BlockClass::transparent) {
                            outMeshData.semiTransparentVertices.push_back(vert);
                            outMeshData.semiTransparentVertexCount++;
                        } else {
                            outMeshData.solidVertices.push_back(vert);
                            outMeshData.solidVertexCount++;
                        }
                    }
                }
            }
        }
    }
}

int32_t ChunkMeshBuilder::estimateVertexCount(const Chunk& chunk) {
    // Simple estimation: count non-air blocks and multiply by average faces
    int32_t nonAirBlocks = 0;
    for (int32_t x = 0; x < Chunk::HorizontalSize; ++x) {
        for (int32_t y = 0; y < Chunk::VerticalSize; ++y) {
            for (int32_t z = 0; z < Chunk::HorizontalSize; ++z) {
                const BlockData* block = chunk.getBlockAt({x, y, z});
                if (block && block->blockClass != BlockData::BlockClass::air) {
                    nonAirBlocks++;
                }
            }
        }
    }
    
    // Assume average of 3 visible faces per block, 4 vertices per face
    return nonAirBlocks * 3 * 4;
}