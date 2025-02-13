#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "ResourceManager/AtlasGenerator.hpp"
#include "Entities/Character.hpp"
#include "World/WorldGenerator.hpp"
#include "World/Chunk.hpp"
#include "Input/InputManager.hpp"
#include "Rendering/Camera.hpp"

namespace Game {

struct HoveredBlock {
	bool valid;
	std::shared_ptr<World::Chunk> chunk;
	int localX, localY, localZ;
	int chunkX, chunkZ;
};

class GameManager {
public:
	GameManager();
	~GameManager();

	void init();
	void handleBlockInteraction(const Engine::Input::InputManager& input, const Engine::Rendering::Camera& camera);
	void update(float dt, const Engine::Input::InputManager& input, const Engine::Rendering::Camera& camera);
	void render(const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight);
	void shutdown();
	void updateHoveredBlock(const Engine::Rendering::Camera& camera);
	void renderBlockHighlight(const glm::mat4& view, const glm::mat4& projection);
	void renderCrosshairTexture(int screenW, int screenH);

	// Accès au player
	Character* getPlayer() const { return m_player.get(); }

private:
	std::unique_ptr<Character> m_player;

	void renderSun(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos);

	// Atlas (textures)
	Atlas m_atlas;

	float m_sunAngle;

	// Shader OpenGL
	GLuint m_shaderProgram;
	GLuint m_2DShaderProgram;
	GLuint m_wireShaderProgram;

	// Vecteur de chunks
	std::vector<std::shared_ptr<World::Chunk>> m_chunks;

	// Méthode interne pour init le shader, l’atlas, etc.
	bool initResources();

	HoveredBlock m_hoveredBlock;
};

} // namespace Game
