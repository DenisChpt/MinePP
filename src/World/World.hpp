/**
 * @class World
 * @brief Représente le monde du jeu, constitué de chunks générés et gérés dynamiquement.
 *
 * @details La classe World gère le chargement/déchargement des chunks en fonction de la position du
 * joueur, la mise à jour du contenu du monde et le rendu en plusieurs passes (opaque et
 * transparent). Elle intègre également la gestion de comportements (WorldBehavior) pour déclencher
 * des effets additionnels lors d'événements sur les blocs (ex. particules).
 */

#pragma once

#include "../Common.hpp"
#include "../Game/Behaviors.hpp"
#include "../Persistence/Persistence.hpp"
#include "../Rendering/Shaders.hpp"
#include "../Rendering/Textures.hpp"
#include "../Utils/Utils.hpp"
#include "Chunk.hpp"
#include "ChunkRegion.hpp"
#include "ChunkMeshTaskManager.hpp"
#include "WorldGenerator.hpp"

#include <Frustum.h>

#include <queue>

class Window;
class Assets;
class Framebuffer;

// ChunkPool for efficient chunk memory management
class ChunkPool {
	std::queue<std::unique_ptr<Chunk>> available;
	static constexpr size_t MAX_SIZE = 100;

   public:
	Ref<Chunk> acquire(glm::ivec2 pos);
	void release(const Ref<Chunk>& chunk);
	size_t size() const { return available.size(); }
};

class World {
	std::unordered_map<glm::ivec2, Ref<Chunk>, Util::HashVec2> chunks;
	std::unordered_map<glm::ivec2, std::unique_ptr<ChunkRegion>, Util::HashVec2> regions;
	std::vector<Ref<WorldBehavior>> behaviors;
	ChunkPool chunkPool;
	std::unique_ptr<ChunkMeshTaskManager> meshTaskManager;
	using ChunkIndexVector = std::vector<std::pair<glm::vec2, float>>;
	Ref<const Texture> textureAtlas;
	Ref<const ShaderProgram> opaqueShader;
	Ref<const ShaderProgram> transparentShader;
	Ref<const ShaderProgram> blendShader;
	bool useAmbientOcclusion = true;

	Window& window;
	Assets& assets;
	Ref<Persistence> persistence;
	WorldGenerator generator;

	const uint32_t MaxRebuildsAllowedPerFrame = 10;

	int32_t viewDistance = 10;
	float textureAnimation = 0;
	static constexpr float TextureAnimationSpeed = 2;

	Ref<Chunk> generateOrLoadChunk(glm::ivec2 position);
	void unloadChunk(const Ref<Chunk>& chunk);
	void sortChunkIndices(glm::vec3 playerPos, const Ref<ChunkIndexVector>& chunkIndices);
	void rebuildChunks(const Ref<ChunkIndexVector>& chunkIndices, const Frustum& frustum);
	
	/**
	 * @brief Manages regions for hierarchical culling
	 */
	void addChunkToRegion(const Ref<Chunk>& chunk);
	void removeChunkFromRegion(const glm::ivec2& chunkPos);
	
	/**
	 * @brief Performs hierarchical frustum culling
	 * @return Number of chunks culled
	 */
	int32_t performHierarchicalCulling(const Frustum& frustum, std::vector<Ref<Chunk>>& visibleChunks);

   public:
	World(Window& window,
		  Assets& assets,
		  const Ref<Persistence>& persistence,
		  std::vector<Ref<WorldBehavior>> behaviors,
		  int32_t seed);
	Window& getWindow() { return window; }
	const Window& getWindow() const { return window; }
	Assets& getAssets() { return assets; }
	const Assets& getAssets() const { return assets; }

	Ref<Chunk> getChunk(glm::ivec2 position);
	void addChunk(glm::ivec2 position, const Ref<Chunk>& chunk);
	[[nodiscard]] static glm::ivec2 getChunkIndex(glm::ivec3 position);

	[[nodiscard]] int32_t getViewDistance() const { return viewDistance; };
	void setViewDistance(int32_t distance) { viewDistance = distance; };

	[[nodiscard]] bool getUseAmbientOcclusion() const { return useAmbientOcclusion; };
	void setUseAmbientOcclusion(bool enabled) { useAmbientOcclusion = enabled; };

	[[nodiscard]] const BlockData* getBlockAt(glm::ivec3 position);
	[[nodiscard]] const BlockData* getBlockAtIfLoaded(glm::ivec3 position) const;
	[[nodiscard]] bool isChunkLoaded(glm::ivec2 position) const;
	bool placeBlock(BlockData block, glm::ivec3 position);

	void update(const glm::vec3& playerPosition, float deltaTime);
	void renderTransparent(glm::mat4 transform,
						   glm::vec3 playerPos,
						   const Frustum& frustum,
						   float zNear,
						   float zFar,
						   const Ref<Framebuffer>& opaqueRender);

	void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum& frustum);
	static bool isValidBlockPosition(glm::ivec3 position);
	void setTextureAtlas(const Ref<const Texture>& texture);

	// ChunkPool stats
	size_t getChunkPoolSize() const { return chunkPool.size(); }
	
	// Mesh task manager stats
	size_t getActiveMeshTasks() const;
	size_t getCompletedMeshTasks() const;
};
