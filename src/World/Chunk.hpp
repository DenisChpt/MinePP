/**
 * @class Chunk
 * @brief Représente un segment du monde (chunk) contenant un volume de blocs et un maillage pour le
 * rendu.
 *
 * @details La classe Chunk stocke un tableau 3D de BlockData et gère la reconstruction du maillage
 *          pour le rendu des blocs opaques et semi-transparents. Elle fournit des méthodes pour
 * placer des blocs, vérifier la visibilité via une AABB et interagir avec le système de
 * persistance.
 *
 * @param worldPosition Coordonnées du chunk dans le monde.
 */

#pragma once

#include "../Common.hpp"
#include "../Math/Math.hpp"
#include "../Rendering/BlockVertex.hpp"
#include "../Rendering/Buffers.hpp"
#include "../Rendering/Mesh.hpp"
#include "../Rendering/Shaders.hpp"
#include "BlockTypes.hpp"
#include "ChunkMeshBuilder.hpp"

#include <Frustum.h>

class Persistence;
class World;

class Chunk {
   public:
	static constexpr int32_t HorizontalSize = 16;
	static constexpr int32_t VerticalSize = 256;

	static constexpr int32_t BlockCount = HorizontalSize * HorizontalSize * VerticalSize;
	static constexpr int32_t MaxVertexCount = BlockCount * 8;
	
	/**
	 * @brief Initial vertex capacity for dynamic allocation
	 * @details Starting with a reasonable capacity based on typical chunk complexity
	 */
	static constexpr int32_t InitialVertexCapacity = 8192;
	
	/**
	 * @brief Growth factor for vertex buffer reallocation
	 */
	static constexpr float VertexBufferGrowthFactor = 1.5f;

   private:
	enum class RenderState { initial, ready, dirty };
	int32_t solidVertexCount;
	int32_t semiTransparentVertexCount;
	Ref<VertexArray> mesh;
	Ref<const ShaderProgram> shader;

	bool useAmbientOcclusion = true;

	RenderState renderState;
	glm::ivec2 worldPosition;

	BlockData data[HorizontalSize][VerticalSize][HorizontalSize];
	AABB aabb;

	/**
	 * @brief Temporary vertex storage for mesh building
	 * 
	 * @details These vectors are used to store vertices during mesh reconstruction,
	 *          replacing static variables to ensure thread safety. They are allocated
	 *          once per chunk and reused for each rebuild operation.
	 */
	Ref<std::vector<BlockVertex>> solidVertices;
	Ref<std::vector<BlockVertex>> semiTransparentVertices;

	/**
	 * @brief Statistics for optimizing memory allocation
	 * @details Track peak usage to optimize future allocations
	 */
	int32_t peakSolidVertexCount = 0;
	int32_t peakSemiTransparentVertexCount = 0;
	int32_t rebuildCount = 0;

	void init();
	void updatePeakUsage();
	void ensureVertexCapacity(int32_t requiredSolidCapacity, int32_t requiredTransparentCapacity);

   public:
	explicit Chunk(const glm::ivec2& worldPosition);

	void renderOpaque(const glm::mat4& transform, const Frustum& frustum);
	void renderSemiTransparent(const glm::mat4& transform, const Frustum& frustum);
	void rebuildMesh(const World& world);
	
	/**
	 * @brief Apply pre-built mesh data to this chunk
	 * 
	 * @details This method is used to apply mesh data that was built
	 *          asynchronously by ChunkMeshBuilder. Must be called from
	 *          the main thread as it uploads data to GPU.
	 * 
	 * @param meshData The mesh data to apply
	 */
	void applyMeshData(const ChunkMeshData& meshData);

	[[nodiscard]] bool needsMeshRebuild() const {
		return !mesh || renderState != RenderState::ready;
	};
	void setShader(const Ref<const ShaderProgram>& newShader) { shader = newShader; };
	void setDirty() { renderState = RenderState::dirty; };
	void setUseAmbientOcclusion(bool enabled) {
		if (enabled == useAmbientOcclusion) {
			return;
		}

		setDirty();
		useAmbientOcclusion = enabled;
	};

	[[nodiscard]] bool isVisible(const Frustum& frustum) const {
		return frustum.IsBoxVisible(aabb.minPoint, aabb.maxPoint);
	};

	void placeBlock(BlockData block, const glm::ivec3& position) {
		placeBlock(block, position.x, position.y, position.z);
	}

	void placeBlock(BlockData block, int32_t x, int32_t y, int32_t z) {
		assert(isInBounds(x, y, z));

		renderState = RenderState::dirty;
		data[x][y][z] = block;
	}

	[[nodiscard]] float distanceToPoint(const glm::vec2& point) const {
		glm::vec2 referencePoint = {
			glm::clamp(point.x, (float)worldPosition.x, (float)worldPosition.x + (float)HorizontalSize),
			glm::clamp(point.y, (float)worldPosition.y, (float)worldPosition.y + (float)HorizontalSize)};

		return glm::distance(referencePoint, point);
	}

	[[nodiscard]] const BlockData* getBlockAt(const glm::ivec3& position) const {
		return &data[position.x][position.y][position.z];
	}

	static bool isInBounds(int32_t x, int32_t y, int32_t z) {
		return x >= 0 && x < HorizontalSize && y >= 0 && y < VerticalSize && z >= 0 &&
			   z < HorizontalSize;
	}

	[[nodiscard]] const BlockData* getBlockAtOptimized(const glm::ivec3& pos,
													   const World& world) const;
	static bool isValidPosition(glm::ivec3 position) {
		return position.y >= 0 && position.y < VerticalSize;
	}
	static glm::ivec3 toChunkCoordinates(const glm::ivec3& globalPosition);

	glm::ivec2 getPosition() { return worldPosition; }

	// Methods for ChunkPool
	void reset(glm::ivec2 newPosition);
	void clear();
	
	/**
	 * @brief Get memory usage statistics
	 * @return Pair of (current total vertex capacity, peak total vertex usage)
	 */
	[[nodiscard]] std::pair<int32_t, int32_t> getMemoryStats() const {
		int32_t currentCapacity = 0;
		if (solidVertices) currentCapacity += solidVertices->capacity();
		if (semiTransparentVertices) currentCapacity += semiTransparentVertices->capacity();
		return {currentCapacity, peakSolidVertexCount + peakSemiTransparentVertexCount};
	}

	friend Persistence;
	friend class ChunkMeshBuilder;
};
