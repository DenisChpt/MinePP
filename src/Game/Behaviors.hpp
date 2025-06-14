// Behaviors.hpp - Consolidation de tous les comportements du monde
#pragma once

#include "../Rendering/Mesh.hpp"
#include "../Rendering/ParticleSystem.hpp"
#include "../Rendering/Shaders.hpp"
#include "../Utils/Utils.hpp"
#include "../World/BlockTypes.hpp"

#include <Frustum.h>

class Assets;
class World;
struct BlockData;

// Interface abstraite WorldBehavior
class WorldBehavior {
   public:
	virtual void onNewBlock(glm::ivec3 blockPos, const BlockData* block, World& world) {}
	virtual void onBlockUpdate(glm::ivec3 blockPos, const BlockData* block, World& world) {}
	virtual void onBlockRemoved(glm::ivec3 blockPos,
								const BlockData* block,
								World& world,
								bool removedByPlayer) {}

	virtual void update(float dt) {}
	virtual void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum& frustum) {}

	virtual ~WorldBehavior() = default;
};

// BlockBreakParticleSystem
class BlockBreakParticleSystem : public ParticleSystem {
	CubeMesh cubeMesh{};
	Ref<const ShaderProgram> cubeShader;

   public:
	BlockBreakParticleSystem(Assets& assets);
	void render(glm::mat4 MVP) override;
};

// LavaParticleSystem
class LavaParticleSystem : public ParticleSystem {
	CubeMesh cubeMesh{};
	Ref<const ShaderProgram> cubeShader;

   public:
	LavaParticleSystem(Assets& assets);
	void render(glm::mat4 MVP) override;
};

// BlockBreakParticleBehavior
class BlockBreakParticleBehavior : public WorldBehavior {
	Random random;
	BlockBreakParticleSystem particleSystem;

	void emitBlockParticle(glm::vec3 pos, glm::vec4 color);

   public:
	BlockBreakParticleBehavior(Assets& assets) : particleSystem(assets) {}
	void onBlockRemoved(glm::ivec3 blockPos,
						const BlockData* block,
						World& world,
						bool removedByPlayer) override;
	void update(float dt) override;
	void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum& frustum) override;
};

// LavaParticleBehavior
class LavaParticleBehavior : public WorldBehavior {
	Random random;
	std::set<glm::ivec3, Util::CompareIVec3> surfaceLavaPositions{};
	LavaParticleSystem particleSystem;
	float emitAttemptFrequency = 0.1;  // seconds
	float timeUntilNextEmit = emitAttemptFrequency;

	void emitLavaParticles(glm::ivec3 pos);
	void emitLavaParticle(glm::vec3 pos);

   public:
	LavaParticleBehavior(Assets& assets) : particleSystem(assets) {}
	void onNewBlock(glm::ivec3 blockPos, const BlockData* block, World& world) override;
	void onBlockUpdate(glm::ivec3 blockPos, const BlockData* block, World& world) override;
	void onBlockRemoved(glm::ivec3 blockPos,
						const BlockData* block,
						World& world,
						bool removedByPlayer) override;
	void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum& frustum) override;
	void update(float dt) override;
};