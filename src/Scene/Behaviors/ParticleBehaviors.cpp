// Nouveau fichier : consolidation des comportements de particules
#include "ParticleBehaviors.hpp"
#include "../../Core/Assets.hpp"

// BlockBreakParticleSystem
BlockBreakParticleSystem::BlockBreakParticleSystem(Assets& assets)
	: cubeShader(assets.loadShaderProgram("assets/shaders/colored_cube_opaque")) {}

void BlockBreakParticleSystem::render(glm::mat4 MVP)
{
	cubeShader->bind();
	for (const auto &particle : particles)
	{
		cubeShader->setVec4("color", particle.color);
		cubeShader->setMat4("MVP", MVP * glm::translate(particle.position) * glm::scale(particle.scale) *
									   glm::rotate(glm::two_pi<float>(), particle.rotation));
		cubeMesh.render();
	}
}

// LavaParticleSystem
LavaParticleSystem::LavaParticleSystem(Assets& assets)
	: cubeShader(assets.loadShaderProgram("assets/shaders/colored_cube_opaque")) {}

void LavaParticleSystem::render(glm::mat4 MVP)
{
	cubeShader->bind();
	for (const auto &particle : particles)
	{
		cubeShader->setVec4("color", particle.color);
		cubeShader->setMat4("MVP", MVP * glm::translate(particle.position) * glm::scale(particle.scale) *
									   glm::rotate(glm::two_pi<float>(), particle.rotation));
		cubeMesh.render();
	}
}

// BlockBreakParticleBehavior
void BlockBreakParticleBehavior::onBlockRemoved(glm::ivec3 blockPos,
												const BlockData *block,
												World &world,
												bool removedByPlayer)
{
	if (!removedByPlayer)
		return;
	if (block == nullptr || block->type == BlockData::BlockType::air)
		return;

	auto color = block->getColor();
	for (int i = 0; i < 50; ++i)
	{
		emitBlockParticle(glm::vec3(blockPos) + random.getVec3(), color);
	}
}

void BlockBreakParticleBehavior::emitBlockParticle(glm::vec3 pos, glm::vec4 color)
{
	particleSystem.emit({
		.position = pos,
		.scale = glm::vec3(0.0625),
		.scaleVelocity = glm::vec3(-0.0625),
		.scaleVelocityVariation = glm::vec3(.05),
		.velocity = glm::vec3(1, 0, 1),
		.velocityVariation = glm::vec3(-2, 1, -2),
		.gravity = glm::vec3(0, -9.8, 0),
		.rotation = glm::vec3(0),
		.angularVelocity = glm::vec3(1, -1, 1),
		.angularVelocityVariation = glm::vec3(.25, .25, .25),
		.startColor = color,
		.endColor = color,
		.startLifetime = .200,
		.lifetimeVariation = .200,
		.lifetime = .200,
	});
}

void BlockBreakParticleBehavior::update(float dt)
{
	particleSystem.update(dt);
}

void BlockBreakParticleBehavior::renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum &frustum)
{
	particleSystem.render(transform);
}

// LavaParticleBehavior
void LavaParticleBehavior::onNewBlock(glm::ivec3 blockPos, const BlockData *block, World &world)
{
	if (block == nullptr || block->type != BlockData::BlockType::lava)
		return;

	auto blockAbove = world.getBlockAtIfLoaded(blockPos + glm::ivec3(0, 1, 0));
	if (blockAbove != nullptr && blockAbove->type == BlockData::BlockType::air)
	{
		surfaceLavaPositions.emplace(blockPos);
	}
}

void LavaParticleBehavior::onBlockUpdate(glm::ivec3 blockPos, const BlockData *block, World &world)
{
	if (block == nullptr || block->type != BlockData::BlockType::lava)
		return;
	auto blockAbove = world.getBlockAtIfLoaded(blockPos + glm::ivec3(0, 1, 0));
	if (blockAbove == nullptr || blockAbove->type == BlockData::BlockType::air)
	{
		surfaceLavaPositions.emplace(blockPos);
	}
	else
	{
		surfaceLavaPositions.erase(blockPos);
	}
}

void LavaParticleBehavior::onBlockRemoved(glm::ivec3 blockPos,
										  const BlockData *block,
										  World &world,
										  bool removedByPlayer)
{
	if (block == nullptr || block->type != BlockData::BlockType::lava)
		return;

	surfaceLavaPositions.erase(blockPos);
}

void LavaParticleBehavior::update(float dt)
{
	timeUntilNextEmit -= dt;
	if (timeUntilNextEmit <= 0)
	{
		timeUntilNextEmit = emitAttemptFrequency;
		for (auto lavaPosition : surfaceLavaPositions)
		{
			if (random.getFloat() < 0.99f)
				continue;
			emitLavaParticles(lavaPosition);
		}
	}
	particleSystem.update(dt);
}

void LavaParticleBehavior::emitLavaParticles(glm::ivec3 pos)
{
	float particlesToEmit = glm::log2(random.getFloat() * 8 + 0.01);
	for (float i = 0; i < particlesToEmit; ++i)
	{
		const glm::vec2 posOnBlock = random.getVec2();
		emitLavaParticle(glm::vec3(pos) + glm::vec3(posOnBlock.x, 1, posOnBlock.y));
	}
}

void LavaParticleBehavior::emitLavaParticle(glm::vec3 pos)
{
	particleSystem.emit({
		.position = pos,
		.scale = glm::vec3(0.0625),
		.scaleVelocity = glm::vec3(-0.0625),
		.scaleVelocityVariation = glm::vec3(.05),
		.velocity = glm::vec3(2, 3, 2),
		.velocityVariation = glm::vec3(-4, 1, -4),
		.gravity = glm::vec3(0, -9.8, 0),
		.rotation = glm::vec3(0),
		.angularVelocity = glm::vec3(1, -1, 1),
		.angularVelocityVariation = glm::vec3(.25, .25, .25),
		.startColor = glm::vec4(.8, 0, 0, 1),
		.endColor = glm::vec4(.4, 0, 0, 1),
		.startLifetime = .5,
		.lifetimeVariation = .25,
		.lifetime = .5,
	});
}

void LavaParticleBehavior::renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum &frustum)
{
	particleSystem.render(transform);
}