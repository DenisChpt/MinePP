// File: src/World.cpp

#include "World.hpp"

#include "../Application/Window.hpp"
#include "../Core/Assets.hpp"
#include "../Core/PerformanceMonitor.hpp"
#include "ChunkRegion.hpp"
#include "LODLevel.hpp"
#include "../Rendering/Buffers.hpp"
#include "../Rendering/ColorRenderPass.hpp"

#include <ranges>

// ChunkPool implementation
Ref<Chunk> ChunkPool::acquire(glm::ivec2 pos) {
	if (!available.empty()) {
		auto chunk = std::shared_ptr<Chunk>(available.front().release());
		available.pop();
		chunk->reset(pos);
		// Debug: log pool reuse
		// printf("ChunkPool: Reused chunk for position (%d, %d), pool size: %zu\n", pos.x, pos.y,
		// available.size());
		return chunk;
	}
	// Debug: log new allocation
	// printf("ChunkPool: New allocation for position (%d, %d)\n", pos.x, pos.y);
	return std::make_shared<Chunk>(pos);
}

void ChunkPool::release(const Ref<Chunk>& chunk) {
	if (available.size() < MAX_SIZE) {
		chunk->clear();
		available.push(std::unique_ptr<Chunk>(new Chunk(*chunk)));
		// Debug: log pool release
		// printf("ChunkPool: Released chunk, pool size: %zu\n", available.size());
	} else {
		// Debug: log pool full
		// printf("ChunkPool: Pool full, discarding chunk\n");
	}
}

World::World(Window& window,
			 Assets& assets,
			 const Ref<Persistence>& persistence,
			 std::vector<Ref<WorldBehavior>> behaviors,
			 int32_t seed)
	: window(window),
	  assets(assets),
	  behaviors(std::move(behaviors)),
	  persistence(persistence),
	  generator(seed) {
	TRACE_FUNCTION();
	opaqueShader = assets.loadShaderProgram("assets/shaders/world_opaque");
	transparentShader = assets.loadShaderProgram("assets/shaders/world_transparent");
	blendShader = assets.loadShaderProgram("assets/shaders/world_blend");

	// On charge la texture atlas (unique) générée par TextureAtlas
	setTextureAtlas(assets.getAtlasTexture());
	
	// Initialize mesh task manager after World is partially constructed
	meshTaskManager = std::make_unique<ChunkMeshTaskManager>(*this, assets);
	
	// Initialize regions for any existing chunks (from persistence)
	for (const auto& [pos, chunk] : chunks) {
		addChunkToRegion(chunk);
	}
}

Ref<Chunk> World::generateOrLoadChunk(glm::ivec2 position) {
	TRACE_FUNCTION();
	Ref<Chunk> chunk = persistence->getChunk(position);
	if (chunk != nullptr) {
		return chunk;
	}
	chunk = chunkPool.acquire(position);
	generator.populateChunk(chunk);
	persistence->commitChunk(chunk);
	return chunk;
}

void World::unloadChunk(const Ref<Chunk>& chunk) {
	const auto chunkPos = chunk->getPosition();
	removeChunkFromRegion(chunkPos);
	chunks.erase(chunkPos);

	// Informer les WorldBehavior que les blocs de ce chunk sont supprimés
	for (int32_t x = 0; x < Chunk::HorizontalSize; ++x) {
		for (int32_t y = 0; y < Chunk::VerticalSize; ++y) {
			for (int32_t z = 0; z < Chunk::HorizontalSize; ++z) {
				for (const auto& worldBehavior : behaviors) {
					glm::ivec3 blockPos = {x, y, z};
					glm::ivec3 globalBlockPos = blockPos + glm::ivec3(chunkPos.x, 0, chunkPos.y);
					worldBehavior->onBlockRemoved(
						globalBlockPos, chunk->getBlockAt(blockPos), *this, false);
				}
			}
		}
	}

	// Release chunk to pool for reuse
	chunkPool.release(chunk);
}

void World::update(const glm::vec3& playerPosition, float deltaTime) {
	TRACE_FUNCTION();
	PERF_TIMER("World::update");

	// On incrémente le "temps" pour l'animation
	textureAnimation += deltaTime * TextureAnimationSpeed;

	// Déterminer le chunk du joueur
	glm::vec2 playerChunkPosition = getChunkIndex(playerPosition);

	// Décharger les chunks trop lointains
	auto chunksCopy = chunks;
	float unloadDistance = static_cast<float>(viewDistance + 1) * Chunk::HorizontalSize + Chunk::HorizontalSize / 2.0f;
	for (const auto& [chunkPosition, chunk] : chunksCopy) {
		if (glm::abs(glm::distance(glm::vec2(chunkPosition), playerChunkPosition)) >
			unloadDistance) {
			unloadChunk(chunk);
		}
	}

	// Charger de nouveaux chunks si le joueur s’approche
	float loadDistance = static_cast<float>(viewDistance) * 16 + 8.0f;
	for (int32_t i = -viewDistance; i <= viewDistance; i++) {
		for (int32_t j = -viewDistance; j <= viewDistance; j++) {
			glm::ivec2 position = glm::ivec2(i * 16, j * 16) + glm::ivec2(playerChunkPosition);
			if (isChunkLoaded(position))
				continue;

			float distance = glm::abs(glm::distance(glm::vec2(position), playerChunkPosition));
			if (distance <= loadDistance) {
				auto chunk = generateOrLoadChunk(position);
				addChunk(position, chunk);
			}
		}
	}

	// Process completed mesh generation tasks
	// This must be done in the main thread for OpenGL operations
	{
		PERF_TIMER("World::processCompletedMeshTasks");
		meshTaskManager->processCompletedTasks();
	}

	// Update des behaviors (particules, etc.)
	for (auto& behavior : behaviors) {
		behavior->update(deltaTime);
	}
}

void World::sortChunkIndices(glm::vec3 playerPos, const Ref<ChunkIndexVector>& chunkIndices) {
	chunkIndices->clear();
	if (chunkIndices->capacity() < chunks.size()) {
		chunkIndices->reserve(chunks.size());
	}

	glm::vec2 playerXZ = glm::vec2(playerPos.x, playerPos.z);
	for (const auto& [key, value] : chunks) {
		chunkIndices->emplace_back(key, value->distanceToPoint(playerXZ));
	}

	// Tri du plus proche au plus lointain
	std::sort(chunkIndices->begin(), chunkIndices->end(), [](const auto& a, const auto& b) {
		return b.second < a.second;	 // on inverse pour .reverse_view ensuite
	});
}

void World::rebuildChunks(const Ref<ChunkIndexVector>& chunkIndices, const Frustum& frustum) {
	// Submit chunks to the mesh task manager instead of rebuilding directly
	// Iterate from closest to farthest (chunkIndices is sorted in reverse)
	for (auto& index : std::ranges::reverse_view(*chunkIndices)) {
		const auto& chunk = chunks[index.first];
		if (chunk->needsMeshRebuild() && chunk->isVisible(frustum)) {
			// Submit to thread pool for async mesh generation
			meshTaskManager->submitChunk(chunk.get());
		}
	}
}

void World::renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum& frustum) {
	TRACE_FUNCTION();
	PERF_TIMER("World::renderOpaque");

	// 1) Hierarchical culling
	static std::vector<Ref<Chunk>> visibleChunks;
	int32_t culledChunks;
	{
		PERF_TIMER("World::hierarchicalCulling");
		culledChunks = performHierarchicalCulling(frustum, visibleChunks);
	}
	
	// Record metrics
	PerformanceMonitor::getInstance().recordCount("Chunks Visible", visibleChunks.size());
	PerformanceMonitor::getInstance().recordCount("Chunks Culled", culledChunks);
	PerformanceMonitor::getInstance().recordCount("Chunks Loaded", chunks.size());
	PerformanceMonitor::getInstance().recordCount("Chunk Pool Size", chunkPool.size());
	
	// Calculate memory usage
	size_t totalVertexMemory = 0;
	for (const auto& [pos, chunk] : chunks) {
		auto [capacity, peak] = chunk->getMemoryStats();
		totalVertexMemory += capacity * sizeof(BlockVertex);
	}
	PerformanceMonitor::getInstance().recordCount("Vertex Memory (MB)", totalVertexMemory / (1024 * 1024));
	
	// 2) Sort visible chunks by distance
	glm::vec2 playerXZ = glm::vec2(playerPos.x, playerPos.z);
	std::sort(visibleChunks.begin(), visibleChunks.end(), 
		[&playerXZ](const Ref<Chunk>& a, const Ref<Chunk>& b) {
			return a->distanceToPoint(playerXZ) < b->distanceToPoint(playerXZ);
		});
	
	// 3) Submit visible chunks that need rebuilding to mesh task manager
	{
		PERF_TIMER("World::meshSubmit");
		for (const auto& chunk : visibleChunks) {
			// Calculate distance for LOD determination
			float distanceInChunks = chunk->distanceToPoint(playerXZ) / static_cast<float>(Chunk::HorizontalSize);
			LODLevel requiredLOD = LODSelector::selectLOD(distanceInChunks);
			
			// Submit task for the required LOD if not generated
			if (!chunk->isLODGenerated(requiredLOD)) {
				// For now, always generate Full LOD first
				// TODO: Implement proper LOD generation strategy
				if (!chunk->isLODGenerated(LODLevel::Full)) {
					meshTaskManager->submitChunk(chunk.get());
				}
			}
		}
	}
	
	// 4) Process completed mesh tasks (apply generated meshes)
	// Process twice to ensure fast visual updates when blocks are broken
	{
		PERF_TIMER("World::meshApply");
		meshTaskManager->processCompletedTasks();
		// Process again in case new tasks completed while we were processing
		meshTaskManager->processCompletedTasks();
	}
	
	// Record mesh task statistics
	PerformanceMonitor::getInstance().recordCount("Active Mesh Tasks", meshTaskManager->getActiveTaskCount());
	PerformanceMonitor::getInstance().recordCount("Completed Mesh Tasks", meshTaskManager->getCompletedTaskCount());

	int totalFrames = 32;
	int32_t currentFrame = static_cast<int32_t>(textureAnimation) % totalFrames;

	// 5) Configure shader
	opaqueShader->bind();
	if (textureAtlas) {
		opaqueShader->setTexture("atlas", textureAtlas, 0);
	}
	opaqueShader->setUInt("textureAnimation", currentFrame);
	opaqueShader->setVec3("lightDirection", glm::normalize(glm::vec3(1, 1, 1)));

	// 6) Render visible chunks
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (const auto& chunk : visibleChunks) {
		// Select appropriate LOD based on distance
		float distanceInChunks = chunk->distanceToPoint(playerXZ) / static_cast<float>(Chunk::HorizontalSize);
		chunk->selectLOD(distanceInChunks);
		
		chunk->setShader(opaqueShader);
		chunk->setUseAmbientOcclusion(useAmbientOcclusion);

		// Render opaque blocks
		chunk->renderOpaque(transform, frustum);

		// Rendu des blocs "semi-transparents" (ex: verre) qui se dessinent aussi dans ce pass
		chunk->renderSemiTransparent(transform, frustum);
	}

	// Rendu additionnel : behaviors opaques (ex: particules cubiques)
	for (const auto& behavior : behaviors) {
		behavior->renderOpaque(transform, playerPos, frustum);
	}

	glDisable(GL_BLEND);
}

void World::renderTransparent(glm::mat4 transform,
							  glm::vec3 playerPos,
							  const Frustum& frustum,
							  float zNear,
							  float zFar,
							  const Ref<Framebuffer>& opaqueRender) {
	TRACE_FUNCTION();

	// 1) Préparer le framebuffer "accum + revealage"
	auto width = opaqueRender->getWidth();
	auto height = opaqueRender->getHeight();
	static Ref<Framebuffer> framebuffer = nullptr;
	if (!framebuffer || framebuffer->getWidth() != width || framebuffer->getHeight() != height) {
		framebuffer = std::make_shared<Framebuffer>(width, height, false, 2);
	}

	// 2) Use hierarchical culling from opaque pass
	static std::vector<Ref<Chunk>> visibleChunks;
	performHierarchicalCulling(frustum, visibleChunks);
	
	// Sort visible chunks by distance (far to near for transparency)
	glm::vec2 playerXZ = glm::vec2(playerPos.x, playerPos.z);
	std::sort(visibleChunks.begin(), visibleChunks.end(), 
		[&playerXZ](const Ref<Chunk>& a, const Ref<Chunk>& b) {
			return a->distanceToPoint(playerXZ) > b->distanceToPoint(playerXZ);
		});

	// 3) Calcul de la frame courante (même totalFrames = 8, par ex)
	int totalFrames = 8;
	int32_t currentFrame = static_cast<int32_t>(textureAnimation) % totalFrames;

	// 4) Bind du FBO
	window.getFramebufferStack()->push(framebuffer);
	glEnable(GL_BLEND);

	// On configure un blend spécial (Weighted Blended OIT)
	glBlendFunci(0, GL_ONE, GL_ONE);
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	framebuffer->clearColorAttachment(1, glm::vec4(1));

	// 5) Configure shader
	transparentShader->bind();
	if (textureAtlas) {
		transparentShader->setTexture("atlas", textureAtlas, 0);
	}
	transparentShader->setUInt("textureAnimation", currentFrame);
	transparentShader->setVec3("lightDirection", glm::normalize(glm::vec3(1, 1, 1)));
	transparentShader->setFloat("zNear", zNear);
	transparentShader->setFloat("zFar", zFar);
	transparentShader->setTexture("opaqueDepth", opaqueRender->getDepthAttachment(), 1);

	// 6) Dessin des chunks semi-transparents (ex: eau) - only visible chunks
	for (const auto& chunk : visibleChunks) {
		chunk->setShader(transparentShader);
		chunk->setUseAmbientOcclusion(useAmbientOcclusion);
		chunk->renderSemiTransparent(transform, frustum);
	}

	// On pop pour repasser au framebuffer précédent
	window.getFramebufferStack()->pop();

	// 7) Composition finale via world_blend.frag
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ColorRenderPass renderPass(blendShader);
	renderPass.setTexture("accumTexture", framebuffer->getColorAttachment(0), 1);
	renderPass.setTexture("revealageTexture", framebuffer->getColorAttachment(1), 2);
	renderPass.setTexture("opaqueTexture", opaqueRender->getColorAttachment(0), 3);
	renderPass.render();

	glDisable(GL_BLEND);
}

const BlockData* World::getBlockAt(glm::ivec3 position) {
	return getChunk(getChunkIndex(position))->getBlockAt(Chunk::toChunkCoordinates(position));
}

bool World::isValidBlockPosition(glm::ivec3 position) {
	return Chunk::isValidPosition(position);
}

bool World::placeBlock(BlockData block, glm::ivec3 position) {
	TRACE_FUNCTION();
	if (!Chunk::isValidPosition(position)) {
		return false;
	}

	glm::ivec3 positionInChunk = Chunk::toChunkCoordinates(position);
	auto chunk = getChunk(getChunkIndex(position));
	auto oldBlock = chunk->getBlockAt(positionInChunk);

	// Notifier les behaviors qu'un bloc est retiré
	for (const auto& behavior : behaviors) {
		behavior->onBlockRemoved(position, oldBlock, *this, true);
	}

	// Placer le nouveau bloc
	chunk->placeBlock(block, positionInChunk);
	
	// Submit chunk for immediate rebuild
	submitChunkForRebuild(chunk.get());

	// Notifier qu'on a ajouté un bloc
	for (const auto& behavior : behaviors) {
		behavior->onNewBlock(position, &block, *this);
	}

	// Marquer les voisins comme "dirty" si besoin, + onBlockUpdate
	constexpr std::array<glm::ivec3, 6> blocksAround = {
		{{0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}}};
	for (const glm::ivec3& offset : blocksAround) {
		glm::ivec3 neighbor = offset + positionInChunk;
		glm::ivec3 neighborWorldPosition = position + offset;
		if (!Chunk::isInBounds(neighbor.x, neighbor.y, neighbor.z)) {
			const auto& chunkN = getChunk(getChunkIndex(neighborWorldPosition));
			chunkN->setDirty();
			// Also submit neighbor chunk for immediate rebuild
			submitChunkForRebuild(chunkN.get());
		}
		for (const auto& behavior : behaviors) {
			behavior->onBlockUpdate(
				neighborWorldPosition, getBlockAt(neighborWorldPosition), *this);
		}
	}

	return true;
}

glm::ivec2 World::getChunkIndex(glm::ivec3 position) {
	return {position.x - Util::positiveMod(position.x, Chunk::HorizontalSize),
			position.z - Util::positiveMod(position.z, Chunk::HorizontalSize)};
}

Ref<Chunk> World::getChunk(glm::ivec2 position) {
	TRACE_FUNCTION();
	if (!isChunkLoaded(position)) {
		addChunk(position, generateOrLoadChunk(position));
	}
	return chunks.at(position);
}

void World::addChunk(glm::ivec2 position, const Ref<Chunk>& chunk) {
	TRACE_FUNCTION();
	chunks[position] = chunk;
	chunk->setShader(opaqueShader);
	
	// Add to region for hierarchical culling
	addChunkToRegion(chunk);

	// Marquer les voisins comme dirty
	std::array<glm::ivec2, 4> chunksAround = {{{0, 16}, {16, 0}, {0, -16}, {-16, 0}}};
	for (const glm::ivec2& offset : chunksAround) {
		glm::ivec2 neighborPosition = position + offset;
		if (!isChunkLoaded(neighborPosition))
			continue;
		chunks[neighborPosition]->setDirty();
	}

	// Notifier behaviors pour tous les blocs de ce chunk
	for (int32_t x = 0; x < Chunk::HorizontalSize; ++x) {
		for (int32_t y = 0; y < Chunk::VerticalSize; ++y) {
			for (int32_t z = 0; z < Chunk::HorizontalSize; ++z) {
				for (const auto& worldBehavior : behaviors) {
					glm::ivec3 blockPos = {x, y, z};
					glm::ivec3 globalPos = blockPos + glm::ivec3(position.x, 0, position.y);
					worldBehavior->onNewBlock(globalPos, chunk->getBlockAt(blockPos), *this);
				}
			}
		}
	}
}

void World::setTextureAtlas(const Ref<const Texture>& texture) {
	textureAtlas = texture;
	// Don't set texture here - it will be set during rendering when shader is bound
}

const BlockData* World::getBlockAtIfLoaded(glm::ivec3 position) const {
	glm::ivec2 index = getChunkIndex(position);
	if (!isChunkLoaded(index)) {
		return nullptr;
	}
	return chunks.at(index)->getBlockAt(Chunk::toChunkCoordinates(position));
}

bool World::isChunkLoaded(glm::ivec2 position) const {
	return chunks.contains(position);
}

/**
 * @brief Adds a chunk to its corresponding region
 * 
 * @details Creates the region if it doesn't exist yet
 */
void World::addChunkToRegion(const Ref<Chunk>& chunk) {
	glm::ivec2 regionPos = ChunkRegion::chunkToRegionPos(chunk->getPosition());
	
	// Create region if it doesn't exist
	if (!regions.contains(regionPos)) {
		regions[regionPos] = std::make_unique<ChunkRegion>(regionPos);
	}
	
	regions[regionPos]->addChunk(chunk);
}

/**
 * @brief Removes a chunk from its region
 * 
 * @details Removes empty regions to save memory
 */
void World::removeChunkFromRegion(const glm::ivec2& chunkPos) {
	glm::ivec2 regionPos = ChunkRegion::chunkToRegionPos(chunkPos);
	
	auto it = regions.find(regionPos);
	if (it != regions.end()) {
		it->second->removeChunk(chunkPos);
		
		// Remove empty regions
		if (it->second->isEmpty()) {
			regions.erase(it);
		}
	}
}

/**
 * @brief Performs hierarchical frustum culling
 * 
 * @details First culls entire regions, then individual chunks within visible regions
 */
int32_t World::performHierarchicalCulling(const Frustum& frustum, std::vector<Ref<Chunk>>& visibleChunks) {
	TRACE_FUNCTION();
	
	visibleChunks.clear();
	visibleChunks.reserve(chunks.size());
	
	int32_t chunksculled = 0;
	int32_t regionsCulled = 0;
	
	// Debug: check if we have regions
	if (regions.empty()) {
		// Fallback: if no regions, use all chunks
		for (const auto& [pos, chunk] : chunks) {
			if (chunk->isVisible(frustum)) {
				visibleChunks.push_back(chunk);
			} else {
				chunksculled++;
			}
		}
		return chunksculled;
	}
	
	// First pass: cull regions
	for (const auto& [regionPos, region] : regions) {
		if (!region->isVisible(frustum)) {
			// Entire region is not visible
			regionsCulled++;
			chunksculled += region->getChunks().size();
			continue;
		}
		
		// Second pass: check individual chunks in visible regions
		for (const auto& chunk : region->getChunks()) {
			if (chunk->isVisible(frustum)) {
				visibleChunks.push_back(chunk);
			} else {
				chunksculled++;
			}
		}
	}
	
	return chunksculled;
}

size_t World::getActiveMeshTasks() const {
	return meshTaskManager->getActiveTaskCount();
}

size_t World::getCompletedMeshTasks() const {
	return meshTaskManager->getCompletedTaskCount();
}

void World::submitChunkForRebuild(Chunk* chunk) {
	if (chunk && chunk->needsMeshRebuild()) {
		meshTaskManager->submitChunk(chunk);
	}
}
