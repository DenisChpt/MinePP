#include "Chunk.hpp"

#include "../Core/Assets.hpp"
#include "../Core/PerformanceMonitor.hpp"
#include "World.hpp"

Chunk::Chunk(const glm::ivec2& worldPosition)
	: worldPosition(worldPosition), aabb(glm::vec3(0), glm::vec3(0)),
	  solidVertices(std::make_shared<std::vector<BlockVertex>>()),
	  semiTransparentVertices(std::make_shared<std::vector<BlockVertex>>()) {
	// Start with initial capacity instead of max capacity
	solidVertices->reserve(InitialVertexCapacity);
	semiTransparentVertices->reserve(InitialVertexCapacity);
	init();
}

void Chunk::init() {
	// Initialize all LOD data
	for (auto& lod : lodData) {
		lod.solidVertexCount = 0;
		lod.semiTransparentVertexCount = 0;
		lod.mesh = nullptr;
		lod.isGenerated = false;
	}
	
	currentLOD = LODLevel::Full;
	renderState = RenderState::initial;

	glm::vec3 position = glm::vec3(worldPosition.x, 0, worldPosition.y);
	glm::vec3 maxOffset = glm::vec3(HorizontalSize, VerticalSize, HorizontalSize);
	aabb = AABB{position, position + maxOffset};
}

void Chunk::renderOpaque(const glm::mat4& transform, const Frustum& frustum) {
	TRACE_FUNCTION();
	
	const auto& lod = lodData[static_cast<size_t>(currentLOD)];
	if (!lod.mesh || !isVisible(frustum)) {
		return;
	}

	if (!shader)
		return;

	// Don't rebind shader - World already bound it
	// Just set the MVP matrix for this chunk
	shader->setMat4("MVP",
					transform * glm::translate(glm::vec3(worldPosition.x, 0, worldPosition.y)));

	if (lod.solidVertexCount != 0) {
		lod.mesh->renderVertexSubStream(lod.solidVertexCount, 0);
	}
}

void Chunk::renderSemiTransparent(const glm::mat4& transform, const Frustum& frustum) {
	TRACE_FUNCTION();
	
	const auto& lod = lodData[static_cast<size_t>(currentLOD)];
	if (!lod.mesh || !isVisible(frustum)) {
		return;
	}

	if (!shader)
		return;

	// Don't rebind shader - World already bound it
	// Just set the MVP matrix for this chunk
	shader->setMat4("MVP",
					transform * glm::translate(glm::vec3(worldPosition.x, 0, worldPosition.y)));

	glDisable(GL_CULL_FACE);
	if (lod.semiTransparentVertexCount != 0) {
		lod.mesh->renderVertexSubStream(lod.semiTransparentVertexCount, lod.solidVertexCount);
	}
	glEnable(GL_CULL_FACE);
}

const BlockData* Chunk::getBlockAtOptimized(const glm::ivec3& pos, const World& world) const {
	const glm::ivec2& worldPos = worldPosition;
	if (pos.y >= 0 && pos.y < Chunk::VerticalSize) {
		if (pos.x >= 0 && pos.x < Chunk::HorizontalSize && pos.z >= 0 &&
			pos.z < Chunk::HorizontalSize) {
			return &getBlock(pos.x, pos.y, pos.z);
		} else {
			return world.getBlockAtIfLoaded(
				glm::ivec3(pos.x + worldPos.x, pos.y, pos.z + worldPos.y));
		}
	}

	return nullptr;
}

/**
 * @brief Updates peak vertex usage statistics
 * 
 * @details Called after mesh rebuild to track maximum vertex usage.
 *          This information is used to optimize future allocations.
 */
void Chunk::updatePeakUsage() {
	// Update peak usage based on full LOD
	const auto& fullLod = lodData[static_cast<size_t>(LODLevel::Full)];
	peakSolidVertexCount = std::max(peakSolidVertexCount, fullLod.solidVertexCount);
	peakSemiTransparentVertexCount = std::max(peakSemiTransparentVertexCount, fullLod.semiTransparentVertexCount);
	rebuildCount++;
}

/**
 * @brief Ensures vertex vectors have sufficient capacity
 * 
 * @details Implements progressive reallocation strategy to minimize memory usage
 *          while avoiding frequent reallocations. Uses growth factor and peak usage
 *          statistics to determine optimal capacity.
 * 
 * @param requiredSolidCapacity Minimum capacity needed for solid vertices
 * @param requiredTransparentCapacity Minimum capacity needed for transparent vertices
 */
void Chunk::ensureVertexCapacity(int32_t requiredSolidCapacity, int32_t requiredTransparentCapacity) {
	// For solid vertices
	if (solidVertices->capacity() < requiredSolidCapacity) {
		// Use peak usage as a guide for allocation
		int32_t targetCapacity = requiredSolidCapacity;
		if (rebuildCount > 5 && peakSolidVertexCount > 0) {
			// After several rebuilds, use peak usage plus growth factor
			targetCapacity = static_cast<int32_t>(peakSolidVertexCount * VertexBufferGrowthFactor);
		}
		targetCapacity = std::max(targetCapacity, requiredSolidCapacity);
		targetCapacity = std::min(targetCapacity, MaxVertexCount);
		
		solidVertices->reserve(targetCapacity);
	}
	
	// For semi-transparent vertices
	if (semiTransparentVertices->capacity() < requiredTransparentCapacity) {
		int32_t targetCapacity = requiredTransparentCapacity;
		if (rebuildCount > 5 && peakSemiTransparentVertexCount > 0) {
			targetCapacity = static_cast<int32_t>(peakSemiTransparentVertexCount * VertexBufferGrowthFactor);
		}
		targetCapacity = std::max(targetCapacity, requiredTransparentCapacity);
		targetCapacity = std::min(targetCapacity, MaxVertexCount);
		
		semiTransparentVertices->reserve(targetCapacity);
	}
}

/**
 * @brief Checks if a block at the given position is not air
 * 
 * @param pos Position to check (can be outside chunk bounds)
 * @param chunk The chunk containing the block
 * @param world World reference for checking adjacent chunks
 * @return true if the block exists and is not air, false otherwise
 */
bool hasNonAirAt(const glm::ivec3& pos, const Chunk& chunk, const World& world) {
	const BlockData* block = chunk.getBlockAtOptimized(pos, world);
	return block != nullptr && block->blockClass != BlockData::BlockClass::air;
}

/**
 * @brief Calculates ambient occlusion level for a vertex
 * 
 * @details Implements smooth lighting by checking the occlusion from neighboring blocks.
 *          The occlusion level is calculated based on the presence of blocks at the
 *          sides and corner positions relative to the vertex.
 * 
 * @param blockPos Position of the block
 * @param vertOffset Offset of the vertex from the block position
 * @param chunk The chunk containing the block
 * @param world World reference for checking adjacent chunks
 * @return Occlusion level from 0 (fully occluded) to 3 (no occlusion)
 */
uint8_t calculateOcclusionLevel(const glm::ivec3& blockPos,
								const glm::ivec3& vertOffset,
								const Chunk& chunk,
								const World& world) {
	glm::ivec3 direction = glm::sign(glm::vec3(vertOffset) - glm::vec3(.5));

	uint8_t side1 = hasNonAirAt(blockPos + direction * glm::ivec3(1, 1, 0), chunk, world) ? 1 : 0;
	uint8_t side2 = hasNonAirAt(blockPos + direction * glm::ivec3(0, 1, 1), chunk, world) ? 1 : 0;
	if (side1 && side2) {
		return 0;
	}

	uint8_t corner = hasNonAirAt(blockPos + direction * glm::ivec3(1, 1, 1), chunk, world) ? 1 : 0;
	return 3 - (side1 + side2 + corner);
}

/**
 * @brief Rebuilds the mesh for this chunk based on visible block faces
 * 
 * @details This method uses ChunkMeshBuilder to generate vertices in a thread-safe manner.
 *          The generated mesh data is then uploaded to the GPU.
 * 
 * @param world Reference to the world to access neighboring chunks
 * 
 * @note This method should be called when the chunk is marked as dirty
 * @warning This method must be called from the main thread as it uploads to GPU
 */
void Chunk::rebuildMesh(const World& world) {
	TRACE_FUNCTION();
	PERF_TIMER("Chunk::rebuildMesh");

	// Use ChunkMeshBuilder to generate mesh data for full LOD
	ChunkMeshData meshData;
	{
		TRACE_SCOPE("Chunk::rebuildMesh::BuildMesh");
		ChunkMeshBuilder::buildMesh(*this, world, world.getAssets(), useAmbientOcclusion, meshData, LODLevel::Full);
	}
	
	// Apply the mesh data
	applyMeshData(meshData, LODLevel::Full);
}

/**
 * @brief Apply pre-built mesh data to this chunk
 * 
 * @details Copies the mesh data and uploads it to GPU
 */
void Chunk::applyMeshData(const ChunkMeshData& meshData, LODLevel lod) {
	TRACE_FUNCTION();
	
	auto& lodData = this->lodData[static_cast<size_t>(lod)];
	
	// Update counts for this LOD
	lodData.solidVertexCount = meshData.solidVertexCount;
	lodData.semiTransparentVertexCount = meshData.semiTransparentVertexCount;
	
	// Copy mesh data to internal buffers
	{
		TRACE_SCOPE("Chunk::applyMeshData::CopyData");
		ensureVertexCapacity(lodData.solidVertexCount, lodData.semiTransparentVertexCount);
		
		if (lodData.solidVertexCount > 0) {
			solidVertices->resize(lodData.solidVertexCount);
			std::copy(meshData.solidVertices.begin(), 
			         meshData.solidVertices.begin() + lodData.solidVertexCount,
			         solidVertices->begin());
		} else {
			solidVertices->clear();
		}
		
		if (lodData.semiTransparentVertexCount > 0) {
			semiTransparentVertices->resize(lodData.semiTransparentVertexCount);
			std::copy(meshData.semiTransparentVertices.begin(),
			         meshData.semiTransparentVertices.begin() + lodData.semiTransparentVertexCount,
			         semiTransparentVertices->begin());
		} else {
			semiTransparentVertices->clear();
		}
	}
	
	// Update statistics
	updatePeakUsage();
	
	// Upload to GPU
	{
		TRACE_SCOPE("Chunk::applyMeshData::UploadToGPU");
		int32_t vertexCount = lodData.solidVertexCount + lodData.semiTransparentVertexCount;

		if (!lodData.mesh) {
			lodData.mesh = std::make_shared<VertexArray>();
			lodData.mesh->addVertexAttributes(BlockVertex::vertexAttributes(), sizeof(BlockVertex));
		}

		Ref<VertexBuffer> buffer = lodData.mesh->getVertexBuffer();
		
		// Always use bufferDynamicData to avoid issues with bufferDynamicSubData
		int32_t bufferSize = glm::max(vertexCount, glm::min(vertexCount + 1000, MaxVertexCount));
		
		// Create combined buffer
		std::vector<BlockVertex> combinedBuffer;
		combinedBuffer.reserve(bufferSize);
		
		// Add vertices
		if (lodData.solidVertexCount > 0) {
			combinedBuffer.insert(combinedBuffer.end(), 
				solidVertices->begin(), solidVertices->end());
		}
		
		if (lodData.semiTransparentVertexCount > 0) {
			combinedBuffer.insert(combinedBuffer.end(), 
				semiTransparentVertices->begin(), semiTransparentVertices->end());
		}
		
		// Pad to buffer size
		combinedBuffer.resize(bufferSize);
		
		// Upload to GPU
		buffer->bufferDynamicData(combinedBuffer, bufferSize, 0);
		lodData.isGenerated = true;
		
		// Mark as ready if this is the full LOD
		if (lod == LODLevel::Full) {
			renderState = RenderState::ready;
		}
	}
}

void Chunk::selectLOD(float distanceInChunks) {
	currentLOD = LODSelector::selectLOD(distanceInChunks);
}

glm::ivec3 Chunk::toChunkCoordinates(const glm::ivec3& globalPosition) {
	return {Util::positiveMod(globalPosition.x, HorizontalSize),
			globalPosition.y,
			Util::positiveMod(globalPosition.z, HorizontalSize)};
}

/**
 * @brief Resets the chunk for reuse by the ChunkPool
 * 
 * @details Clears all blocks and reinitializes the chunk with a new position.
 *          This method is used by the ChunkPool to recycle chunk objects.
 * 
 * @param newPosition The new world position for this chunk
 */
void Chunk::reset(glm::ivec2 newPosition) {
	worldPosition = newPosition;
	init();

	// Clear all blocks to air
	std::fill(data.begin(), data.end(), BlockData{BlockData::BlockType::air});

	// Ensure vertex vectors are allocated with initial capacity
	if (!solidVertices) {
		solidVertices = std::make_shared<std::vector<BlockVertex>>();
		solidVertices->reserve(InitialVertexCapacity);
	} else {
		// Shrink to initial capacity if much larger
		if (solidVertices->capacity() > InitialVertexCapacity * 4) {
			solidVertices->clear();
			solidVertices->shrink_to_fit();
			solidVertices->reserve(InitialVertexCapacity);
		}
	}
	
	if (!semiTransparentVertices) {
		semiTransparentVertices = std::make_shared<std::vector<BlockVertex>>();
		semiTransparentVertices->reserve(InitialVertexCapacity);
	} else {
		// Shrink to initial capacity if much larger
		if (semiTransparentVertices->capacity() > InitialVertexCapacity * 4) {
			semiTransparentVertices->clear();
			semiTransparentVertices->shrink_to_fit();
			semiTransparentVertices->reserve(InitialVertexCapacity);
		}
	}
	
	// Reset statistics for new chunk
	peakSolidVertexCount = 0;
	peakSemiTransparentVertexCount = 0;
	rebuildCount = 0;
}

/**
 * @brief Clears the chunk's mesh data
 * 
 * @details Resets the render state and releases the mesh resources.
 *          This method is typically called when a chunk is being deallocated
 *          or returned to the pool.
 */
void Chunk::clear() {
	// Reset render state and clear all LOD meshes
	renderState = RenderState::initial;
	
	for (auto& lod : lodData) {
		lod.solidVertexCount = 0;
		lod.semiTransparentVertexCount = 0;
		lod.mesh = nullptr;
		lod.isGenerated = false;
	}
	
	currentLOD = LODLevel::Full;
}
