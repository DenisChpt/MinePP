// File: src/World.cpp

#include "World.hpp"

#include <ranges>

#include "../Application/Window.hpp"
#include "../AssetManager/AssetManager.hpp"
#include "../Rendering/ColorRenderPass.hpp"
#include "../Rendering/Buffers.hpp"
#include "../AssetManager/TextureAtlas.hpp"
#include "../Core/Context.hpp"

World::World(Context& context, const Ref<Persistence> &persistence, std::vector<Ref<WorldBehavior>> behaviors, int32_t seed)
	: context(context),
	  behaviors(std::move(behaviors)),
	  persistence(persistence),
	  generator(seed)
{
	TRACE_FUNCTION();
	opaqueShader = context.getAssetManager().loadShaderProgram("assets/shaders/world_opaque");
	transparentShader = context.getAssetManager().loadShaderProgram("assets/shaders/world_transparent");
	blendShader = context.getAssetManager().loadShaderProgram("assets/shaders/world_blend");

	// On charge la texture atlas (unique) générée par TextureAtlas
	setTextureAtlas(context.getTextureAtlas().getAtlasTexture());
}

Ref<Chunk> World::generateOrLoadChunk(glm::ivec2 position)
{
	TRACE_FUNCTION();
	Ref<Chunk> chunk = persistence->getChunk(position);
	if (chunk != nullptr)
	{
		return chunk;
	}
	chunk = std::make_shared<Chunk>(position);
	generator.populateChunk(chunk);
	persistence->commitChunk(chunk);
	return chunk;
}

void World::unloadChunk(const Ref<Chunk> &chunk)
{
	const auto chunkPos = chunk->getPosition();
	chunks.erase(chunkPos);

	// Informer les WorldBehavior que les blocs de ce chunk sont supprimés
	for (int32_t x = 0; x < Chunk::HorizontalSize; ++x)
	{
		for (int32_t y = 0; y < Chunk::VerticalSize; ++y)
		{
			for (int32_t z = 0; z < Chunk::HorizontalSize; ++z)
			{
				for (const auto &worldBehavior : behaviors)
				{
					glm::ivec3 blockPos = {x, y, z};
					glm::ivec3 globalBlockPos = blockPos + glm::ivec3(chunkPos.x, 0, chunkPos.y);
					worldBehavior->onBlockRemoved(globalBlockPos, chunk->getBlockAt(blockPos), *this, false);
				}
			}
		}
	}
}

void World::update(const glm::vec3 &playerPosition, float deltaTime)
{
	TRACE_FUNCTION();

	// On incrémente le "temps" pour l'animation
	textureAnimation += deltaTime * TextureAnimationSpeed;

	// Déterminer le chunk du joueur
	glm::vec2 playerChunkPosition = getChunkIndex(playerPosition);

	// Décharger les chunks trop lointains
	auto chunksCopy = chunks;
	float unloadDistance = static_cast<float>(viewDistance + 1) * 16 + 8.0f;
	for (const auto &[chunkPosition, chunk] : chunksCopy)
	{
		if (glm::abs(glm::distance(glm::vec2(chunkPosition), playerChunkPosition)) > unloadDistance)
		{
			unloadChunk(chunk);
		}
	}

	// Charger de nouveaux chunks si le joueur s’approche
	float loadDistance = static_cast<float>(viewDistance) * 16 + 8.0f;
	for (int32_t i = -viewDistance; i <= viewDistance; i++)
	{
		for (int32_t j = -viewDistance; j <= viewDistance; j++)
		{
			glm::ivec2 position = glm::ivec2(i * 16, j * 16) + glm::ivec2(playerChunkPosition);
			if (isChunkLoaded(position))
				continue;

			float distance = glm::abs(glm::distance(glm::vec2(position), playerChunkPosition));
			if (distance <= loadDistance)
			{
				addChunk(position, generateOrLoadChunk(position));
			}
		}
	}

	// Update des behaviors (particules, etc.)
	for (auto &behavior : behaviors)
	{
		behavior->update(deltaTime);
	}
}

void World::sortChunkIndices(glm::vec3 playerPos, const Ref<ChunkIndexVector> &chunkIndices)
{
	chunkIndices->clear();
	if (chunkIndices->capacity() < chunks.size())
	{
		chunkIndices->reserve(chunks.size());
	}

	glm::vec2 playerXZ = glm::vec2(playerPos.x, playerPos.z);
	for (const auto &[key, value] : chunks)
	{
		chunkIndices->emplace_back(key, value->distanceToPoint(playerXZ));
	}

	// Tri du plus proche au plus lointain
	std::sort(chunkIndices->begin(), chunkIndices->end(),
			  [](const auto &a, const auto &b)
			  {
				  return b.second < a.second; // on inverse pour .reverse_view ensuite
			  });
}

void World::rebuildChunks(const Ref<ChunkIndexVector> &chunkIndices, const Frustum &frustum)
{
	uint32_t meshesRebuilt = 0;

	// On itère du plus proche au plus lointain (car chunkIndices est trié à l'envers + .reverse_view)
	for (auto &index : std::ranges::reverse_view(*chunkIndices))
	{
		if (meshesRebuilt > MaxRebuildsAllowedPerFrame)
		{
			break;
		}
		const auto &chunk = chunks[index.first];
		if (chunk->needsMeshRebuild() && chunk->isVisible(frustum))
		{
			chunk->rebuildMesh(*this);
			meshesRebuilt++;
		}
	}
}

void World::renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum &frustum)
{
	TRACE_FUNCTION();

	// 1) Tri et rebuild des chunks au besoin
	static auto sortedChunkIndices = std::make_shared<ChunkIndexVector>();
	sortChunkIndices(playerPos, sortedChunkIndices);
	rebuildChunks(sortedChunkIndices, frustum);

	int totalFrames = 32;
	int32_t currentFrame = static_cast<int32_t>(textureAnimation) % totalFrames;

	// 3) Envoi au shader
	opaqueShader->setUInt("textureAnimation", currentFrame);

	// 4) Rendu
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (const auto &index : *sortedChunkIndices)
	{
		const auto &chunk = chunks[index.first];
		chunk->setShader(opaqueShader);
		chunk->setUseAmbientOcclusion(useAmbientOcclusion);

		// Rendu des blocs opaques
		chunk->renderOpaque(transform, frustum);

		// Rendu des blocs "semi-transparents" (ex: verre) qui se dessinent aussi dans ce pass
		chunk->renderSemiTransparent(transform, frustum);
	}

	// Rendu additionnel : behaviors opaques (ex: particules cubiques)
	for (const auto &behavior : behaviors)
	{
		behavior->renderOpaque(transform, playerPos, frustum);
	}

	glDisable(GL_BLEND);
}

void World::renderTransparent(glm::mat4 transform,
							  glm::vec3 playerPos,
							  const Frustum &frustum,
							  float zNear,
							  float zFar,
							  const Ref<Framebuffer> &opaqueRender)
{
	TRACE_FUNCTION();

	// 1) Préparer le framebuffer "accum + revealage"
	auto width = opaqueRender->getWidth();
	auto height = opaqueRender->getHeight();
	static Ref<Framebuffer> framebuffer = nullptr;
	if (!framebuffer || framebuffer->getWidth() != width || framebuffer->getHeight() != height)
	{
		framebuffer = std::make_shared<Framebuffer>(width, height, false, 2);
	}

	// 2) Tri + rebuild
	static auto sortedChunkIndices = std::make_shared<ChunkIndexVector>();
	sortChunkIndices(playerPos, sortedChunkIndices);
	rebuildChunks(sortedChunkIndices, frustum);

	// 3) Calcul de la frame courante (même totalFrames = 8, par ex)
	int totalFrames = 8;
	int32_t currentFrame = static_cast<int32_t>(textureAnimation) % totalFrames;

	// 4) Bind du FBO
	context.getWindow().getFramebufferStack()->push(framebuffer);
	glEnable(GL_BLEND);

	// On configure un blend spécial (Weighted Blended OIT)
	glBlendFunci(0, GL_ONE, GL_ONE);
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	framebuffer->clearColorAttachment(1, glm::vec4(1));

	// 5) Paramètres du shader transparent
	transparentShader->setUInt("textureAnimation", currentFrame);
	transparentShader->setFloat("zNear", zNear);
	transparentShader->setFloat("zFar", zFar);
	transparentShader->setTexture("opaqueDepth", opaqueRender->getDepthAttachment(), 1);
	transparentShader->bind();

	// 6) Dessin des chunks semi-transparents (ex: eau)
	for (const auto &[key, chunk] : chunks)
	{
		chunk->setShader(transparentShader);
		chunk->setUseAmbientOcclusion(useAmbientOcclusion);
		chunk->renderSemiTransparent(transform, frustum);
	}

	// On pop pour repasser au framebuffer précédent
	context.getWindow().getFramebufferStack()->pop();

	// 7) Composition finale via world_blend.frag
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ColorRenderPass renderPass(blendShader);
	renderPass.setTexture("accumTexture", framebuffer->getColorAttachment(0), 1);
	renderPass.setTexture("revealageTexture", framebuffer->getColorAttachment(1), 2);
	renderPass.setTexture("opaqueTexture", opaqueRender->getColorAttachment(0), 3);
	renderPass.render();

	glDisable(GL_BLEND);
}

const BlockData *World::getBlockAt(glm::ivec3 position)
{
	return getChunk(getChunkIndex(position))->getBlockAt(Chunk::toChunkCoordinates(position));
}

bool World::isValidBlockPosition(glm::ivec3 position)
{
	return Chunk::isValidPosition(position);
}

bool World::placeBlock(BlockData block, glm::ivec3 position)
{
	TRACE_FUNCTION();
	if (!Chunk::isValidPosition(position))
	{
		return false;
	}

	glm::ivec3 positionInChunk = Chunk::toChunkCoordinates(position);
	auto chunk = getChunk(getChunkIndex(position));
	auto oldBlock = chunk->getBlockAt(positionInChunk);

	// Notifier les behaviors qu'un bloc est retiré
	for (const auto &behavior : behaviors)
	{
		behavior->onBlockRemoved(position, oldBlock, *this, true);
	}

	// Placer le nouveau bloc
	chunk->placeBlock(block, positionInChunk);

	// Notifier qu'on a ajouté un bloc
	for (const auto &behavior : behaviors)
	{
		behavior->onNewBlock(position, &block, *this);
	}

	// Marquer les voisins comme "dirty" si besoin, + onBlockUpdate
	constexpr std::array<glm::ivec3, 6> blocksAround = {
		{{0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}}};
	for (const glm::ivec3 &offset : blocksAround)
	{
		glm::ivec3 neighbor = offset + positionInChunk;
		glm::ivec3 neighborWorldPosition = position + offset;
		if (!Chunk::isInBounds(neighbor.x, neighbor.y, neighbor.z))
		{
			const auto &chunkN = getChunk(getChunkIndex(neighborWorldPosition));
			chunkN->setDirty();
		}
		for (const auto &behavior : behaviors)
		{
			behavior->onBlockUpdate(neighborWorldPosition, getBlockAt(neighborWorldPosition), *this);
		}
	}

	return true;
}

glm::ivec2 World::getChunkIndex(glm::ivec3 position)
{
	return {
		position.x - Util::positiveMod(position.x, Chunk::HorizontalSize),
		position.z - Util::positiveMod(position.z, Chunk::HorizontalSize)};
}

Ref<Chunk> World::getChunk(glm::ivec2 position)
{
	TRACE_FUNCTION();
	if (!isChunkLoaded(position))
	{
		addChunk(position, generateOrLoadChunk(position));
	}
	return chunks.at(position);
}

void World::addChunk(glm::ivec2 position, const Ref<Chunk> &chunk)
{
	TRACE_FUNCTION();
	chunks[position] = chunk;

	// Marquer les voisins comme dirty
	std::array<glm::ivec2, 4> chunksAround = {{{0, 16}, {16, 0}, {0, -16}, {-16, 0}}};
	for (const glm::ivec2 &offset : chunksAround)
	{
		glm::ivec2 neighborPosition = position + offset;
		if (!isChunkLoaded(neighborPosition))
			continue;
		chunks[neighborPosition]->setDirty();
	}

	// Notifier behaviors pour tous les blocs de ce chunk
	for (int32_t x = 0; x < Chunk::HorizontalSize; ++x)
	{
		for (int32_t y = 0; y < Chunk::VerticalSize; ++y)
		{
			for (int32_t z = 0; z < Chunk::HorizontalSize; ++z)
			{
				for (const auto &worldBehavior : behaviors)
				{
					glm::ivec3 blockPos = {x, y, z};
					glm::ivec3 globalPos = blockPos + glm::ivec3(position.x, 0, position.y);
					worldBehavior->onNewBlock(globalPos, chunk->getBlockAt(blockPos), *this);
				}
			}
		}
	}
}

void World::setTextureAtlas(const Ref<const Texture> &texture)
{
	textureAtlas = texture;
	opaqueShader->setTexture("atlas", textureAtlas, 0);
	transparentShader->setTexture("atlas", textureAtlas, 0);
}

const BlockData *World::getBlockAtIfLoaded(glm::ivec3 position) const
{
	glm::ivec2 index = getChunkIndex(position);
	if (!isChunkLoaded(index))
	{
		return nullptr;
	}
	return chunks.at(index)->getBlockAt(Chunk::toChunkCoordinates(position));
}

bool World::isChunkLoaded(glm::ivec2 position) const
{
	return chunks.contains(position);
}
