#include "Chunk.hpp"

#include "../Core/Assets.hpp"
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
	solidVertexCount = 0;
	semiTransparentVertexCount = 0;
	mesh = nullptr;
	renderState = RenderState::initial;

	glm::vec3 position = glm::vec3(worldPosition.x, 0, worldPosition.y);
	glm::vec3 maxOffset = glm::vec3(HorizontalSize, VerticalSize, HorizontalSize);
	aabb = AABB{position, position + maxOffset};
}

void Chunk::renderOpaque(const glm::mat4& transform, const Frustum& frustum) {
	TRACE_FUNCTION();
	if (!mesh || !isVisible(frustum)) {
		return;
	}

	if (!shader)
		return;

	// Don't rebind shader - World already bound it
	// Just set the MVP matrix for this chunk
	shader->setMat4("MVP",
					transform * glm::translate(glm::vec3(worldPosition.x, 0, worldPosition.y)));

	if (solidVertexCount != 0) {
		mesh->renderVertexSubStream(solidVertexCount, 0);
	}
}

void Chunk::renderSemiTransparent(const glm::mat4& transform, const Frustum& frustum) {
	TRACE_FUNCTION();
	if (!mesh || !isVisible(frustum)) {
		return;
	}

	if (!shader)
		return;

	// Don't rebind shader - World already bound it
	// Just set the MVP matrix for this chunk
	shader->setMat4("MVP",
					transform * glm::translate(glm::vec3(worldPosition.x, 0, worldPosition.y)));

	glDisable(GL_CULL_FACE);
	if (semiTransparentVertexCount != 0) {
		mesh->renderVertexSubStream(semiTransparentVertexCount, solidVertexCount);
	}
	glEnable(GL_CULL_FACE);
}

const BlockData* Chunk::getBlockAtOptimized(const glm::ivec3& pos, const World& world) const {
	const glm::ivec2& worldPos = worldPosition;
	if (pos.y >= 0 && pos.y < Chunk::VerticalSize) {
		if (pos.x >= 0 && pos.x < Chunk::HorizontalSize && pos.z >= 0 &&
			pos.z < Chunk::HorizontalSize) {
			return &data[pos.x][pos.y][pos.z];
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
	peakSolidVertexCount = std::max(peakSolidVertexCount, solidVertexCount);
	peakSemiTransparentVertexCount = std::max(peakSemiTransparentVertexCount, semiTransparentVertexCount);
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
 * @details This method iterates through all blocks in the chunk and generates
 *          vertices for faces that are exposed (not occluded by neighboring blocks).
 *          It separates solid and semi-transparent geometry for proper rendering order.
 *          The method also applies ambient occlusion if enabled.
 * 
 * @param world Reference to the world to access neighboring chunks
 * 
 * @note This method should be called when the chunk is marked as dirty
 * @warning This is a performance-critical method that should be optimized
 */
void Chunk::rebuildMesh(const World& world) {
	TRACE_FUNCTION();

	solidVertexCount = 0;
	semiTransparentVertexCount = 0;

	const std::array<glm::ivec3, 6> offsetsToCheck = {{
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
		{0, 0, 1},
		{0, 0, -1},
	}};
	{
		TRACE_SCOPE("Chunk::rebuildMesh::WalkBlocks");
		for (int32_t x = HorizontalSize - 1; x >= 0; --x) {
			for (int32_t y = VerticalSize - 1; y >= 0; --y) {
				for (int32_t z = HorizontalSize - 1; z >= 0; --z) {
					glm::ivec3 blockPos = {x, y, z};
					const auto& [type, blockClass] = data[x][y][z];
					if (blockClass == BlockData::BlockClass::air) {
						continue;
					}

					for (const glm::ivec3& offset : offsetsToCheck) {
						const BlockData* block = getBlockAtOptimized(blockPos + offset, world);
						if (block == nullptr) {
							continue;
						}

						bool isSameClass = block->blockClass == blockClass;
						bool isTransparentNextToOpaque =
							block->blockClass == BlockData::BlockClass::solid &&
							blockClass == BlockData::BlockClass::transparent;
						if (isSameClass || isTransparentNextToOpaque) {
							continue;
						}

						for (const auto& vertex : BlockMesh::getVerticesFromDirection(offset)) {
							BlockVertex vert = vertex;
							vert.offset(x, y, z);
							vert.setType(offset, type, world.getAssets());

							uint8_t occlusionLevel = 3;
							if (useAmbientOcclusion) {
								if (offset.y == -1) {
									occlusionLevel = 0;
								} else {
									occlusionLevel = calculateOcclusionLevel(
										blockPos, vert.getPosition() - blockPos, *this, world);
								}
							}
							vert.setOcclusionLevel(occlusionLevel);

							if (blockClass == BlockData::BlockClass::semiTransparent ||
								blockClass == BlockData::BlockClass::transparent) {
								// Ensure capacity before adding
								if (semiTransparentVertexCount >= semiTransparentVertices->size()) {
									ensureVertexCapacity(solidVertexCount, semiTransparentVertexCount + 1000);
									semiTransparentVertices->resize(semiTransparentVertices->capacity());
								}
								(*semiTransparentVertices)[semiTransparentVertexCount] = vert;
								semiTransparentVertexCount++;
							} else {
								// Ensure capacity before adding
								if (solidVertexCount >= solidVertices->size()) {
									ensureVertexCapacity(solidVertexCount + 1000, semiTransparentVertexCount);
									solidVertices->resize(solidVertices->capacity());
								}
								(*solidVertices)[solidVertexCount] = vert;
								solidVertexCount++;
							}
						}
					}
				}
			}
		}
	}
	
	// Update statistics
	updatePeakUsage();
	
	{
		TRACE_SCOPE("Chunk::rebuildMesh::UploadToGPU");
		int32_t vertexCount = solidVertexCount + semiTransparentVertexCount;

		if (!mesh) {
			mesh = std::make_shared<VertexArray>();
			mesh->addVertexAttributes(BlockVertex::vertexAttributes(), sizeof(BlockVertex));
		}

		Ref<VertexBuffer> buffer = mesh->getVertexBuffer();
		
		// Always use bufferDynamicData to avoid issues with bufferDynamicSubData
		// The overhead is minimal compared to mesh generation
		int32_t bufferSize = glm::max(vertexCount, glm::min(vertexCount + 1000, MaxVertexCount));
		
		// Create combined buffer
		std::vector<BlockVertex> combinedBuffer;
		combinedBuffer.reserve(bufferSize);
		
		// Add solid vertices
		if (solidVertexCount > 0) {
			combinedBuffer.insert(combinedBuffer.end(), 
				solidVertices->begin(), solidVertices->begin() + solidVertexCount);
		}
		
		// Add semi-transparent vertices
		if (semiTransparentVertexCount > 0) {
			combinedBuffer.insert(combinedBuffer.end(), 
				semiTransparentVertices->begin(), semiTransparentVertices->begin() + semiTransparentVertexCount);
		}
		
		// Pad to buffer size
		combinedBuffer.resize(bufferSize);
		
		// Upload to GPU
		buffer->bufferDynamicData(combinedBuffer, bufferSize, 0);
		renderState = RenderState::ready;
	}
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
	for (int32_t x = 0; x < HorizontalSize; ++x) {
		for (int32_t y = 0; y < VerticalSize; ++y) {
			for (int32_t z = 0; z < HorizontalSize; ++z) {
				data[x][y][z] = BlockData{BlockData::BlockType::air};
			}
		}
	}

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
	// Reset render state and clear mesh
	renderState = RenderState::initial;
	solidVertexCount = 0;
	semiTransparentVertexCount = 0;
	mesh = nullptr;
}
