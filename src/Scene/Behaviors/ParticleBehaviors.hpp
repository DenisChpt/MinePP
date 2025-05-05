// Nouveau fichier : headers consolidés pour les behaviors de particules
#pragma once
#include "../../Rendering/ParticleSystem.hpp"
#include "../../World/World.hpp"
#include "../CubeMesh.hpp"

// BlockBreakParticleSystem
class BlockBreakParticleSystem : public ParticleSystem
{
	CubeMesh cubeMesh{};
	Ref<const ShaderProgram> cubeShader;

public:
	BlockBreakParticleSystem();
	void render(glm::mat4 MVP) override;
};

// LavaParticleSystem
class LavaParticleSystem : public ParticleSystem
{
	CubeMesh cubeMesh{};
	Ref<const ShaderProgram> cubeShader;

public:
	LavaParticleSystem();
	void render(glm::mat4 MVP) override;
};

// BlockBreakParticleBehavior
class BlockBreakParticleBehavior : public WorldBehavior
{
	Random random;
	BlockBreakParticleSystem particleSystem;

	void emitBlockParticle(glm::vec3 pos, glm::vec4 color);

public:
	void onBlockRemoved(glm::ivec3 blockPos, const BlockData *block, World &world, bool removedByPlayer) override;
	void update(float dt) override;
	void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum &frustum) override;
};

// LavaParticleBehavior
class LavaParticleBehavior : public WorldBehavior
{
	Random random;
	std::set<glm::ivec3, Util::CompareIVec3> surfaceLavaPositions{};
	LavaParticleSystem particleSystem;
	float emitAttemptFrequency = 0.1; // seconds
	float timeUntilNextEmit = emitAttemptFrequency;

	void emitLavaParticles(glm::ivec3 pos);
	void emitLavaParticle(glm::vec3 pos);

public:
	LavaParticleBehavior() = default;
	void onNewBlock(glm::ivec3 blockPos, const BlockData *block, World &world) override;
	void onBlockUpdate(glm::ivec3 blockPos, const BlockData *block, World &world) override;
	void onBlockRemoved(glm::ivec3 blockPos, const BlockData *block, World &world, bool removedByPlayer) override;
	void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum &frustum) override;
	void update(float dt) override;
};