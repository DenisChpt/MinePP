/**
 * @class WorldBehavior
 * @brief Interface abstraite permettant de définir des comportements réactifs aux changements dans le monde.
 *
 * @details Les classes dérivées de WorldBehavior (par exemple, BlockBreakParticleBehavior et LavaParticleBehavior)
 *          implémentent des méthodes pour réagir à l'ajout, la mise à jour ou la suppression de blocs et pour lancer des effets (particules,
 *          animations, etc.).
 */


#pragma once

#include <Frustum.h>

#include "../../World/BlockTypes.hpp"

class World;
struct BlockData;
class WorldBehavior
{
public:
	virtual void onNewBlock(glm::ivec3 blockPos, const BlockData *block, World &world) {}
	virtual void onBlockUpdate(glm::ivec3 blockPos, const BlockData *block, World &world) {}
	virtual void onBlockRemoved(glm::ivec3 blockPos, const BlockData *block, World &world, bool removedByPlayer) {}

	virtual void update(float dt) {}
	virtual void renderOpaque(glm::mat4 transform, glm::vec3 playerPos, const Frustum &frustum) {}

	virtual ~WorldBehavior() = default;
};
