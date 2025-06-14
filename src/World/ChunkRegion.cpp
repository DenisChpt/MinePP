#include "ChunkRegion.hpp"

ChunkRegion::ChunkRegion(const glm::ivec2& regionPos) 
    : regionPosition(regionPos),
      boundingBox(
          glm::vec3(
              regionPosition.x * RegionSize * Chunk::HorizontalSize, 
              0, 
              regionPosition.y * RegionSize * Chunk::HorizontalSize
          ),
          glm::vec3(
              (regionPosition.x + 1) * RegionSize * Chunk::HorizontalSize, 
              Chunk::VerticalSize, 
              (regionPosition.y + 1) * RegionSize * Chunk::HorizontalSize
          )
      ) {
}

void ChunkRegion::addChunk(const Ref<Chunk>& chunk) {
    assert(containsChunk(chunk->getPosition()) && "Chunk does not belong to this region");
    
    // Check if chunk already exists
    auto it = std::find_if(chunks.begin(), chunks.end(),
        [&chunk](const Ref<Chunk>& existing) {
            return existing->getPosition() == chunk->getPosition();
        });
    
    if (it == chunks.end()) {
        chunks.push_back(chunk);
        dirty = true;
    }
}

bool ChunkRegion::removeChunk(const glm::ivec2& chunkWorldPos) {
    auto it = std::find_if(chunks.begin(), chunks.end(),
        [&chunkWorldPos](const Ref<Chunk>& chunk) {
            return chunk->getPosition() == chunkWorldPos;
        });
    
    if (it != chunks.end()) {
        chunks.erase(it);
        dirty = true;
        return true;
    }
    return false;
}

void ChunkRegion::updateBoundingBox() {
    if (!dirty || chunks.empty()) {
        return;
    }
    
    // For now, we use the full region bounds
    // In the future, we could calculate tighter bounds based on actual chunk heights
    dirty = false;
}