/**
 * @class ParticleSystem
 * @brief Système abstrait de particules pour gérer les effets visuels dynamiques.
 *
 * @details La classe ParticleSystem gère l'émission, la mise à jour et le rendu des particules.
 *          Elle sert de base pour les systèmes spécifiques comme BlockBreakParticleSystem et
 * LavaParticleSystem.
 *
 * @note La méthode render() est pure virtuelle et doit être implémentée dans les classes dérivées.
 */

#pragma once

#include "../Common.hpp"
#include "../Utils/Utils.hpp"
#include "InstancedParticleRenderer.hpp"

struct ParticleDescription {
	glm::vec3 position{0, 0, 0};

	glm::vec3 scale{1, 1, 1};
	glm::vec3 scaleVelocity{1, 1, 1};
	glm::vec3 scaleVelocityVariation{1, 1, 1};

	glm::vec3 velocity{0, 1, 0};
	glm::vec3 velocityVariation{1, 1, 1};
	glm::vec3 gravity{0, -9.81, 0};

	glm::vec3 rotation{0, 0, 0};
	glm::vec3 angularVelocity{0, 0, 0};
	glm::vec3 angularVelocityVariation{0, 0, 0};

	glm::vec4 startColor{1, 1, 1, 1};
	glm::vec4 endColor{.5, .5, .5, 1};

	glm::vec4 color;
	float startLifetime;
	float lifetimeVariation{.5};
	float lifetime{1};
};

class ParticleSystem {
   protected:
	Random random;
	std::vector<ParticleDescription> particles;
	
	// Prepare instance data for rendering
	std::vector<ParticleInstanceData> prepareInstanceData(const glm::mat4& viewProjection) const;

   public:
	ParticleSystem() = default;

	void update(float deltaTime);
	virtual void render(glm::mat4 MVP) = 0;
	void emit(const ParticleDescription& particle);
	
	// Get current particle count
	size_t getParticleCount() const { return particles.size(); }

	virtual ~ParticleSystem() = default;
};
