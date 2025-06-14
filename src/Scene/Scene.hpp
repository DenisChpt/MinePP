// Scene.hpp refactorisé - intégration de Skybox et BlockOutline
#pragma once

#include "../Common.hpp"
#include "../Game/Effects.hpp"	// Un seul include pour tous les effets
#include "../Persistence/Persistence.hpp"
#include "../Rendering/Buffers.hpp"
#include "../Rendering/Mesh.hpp"
#include "../Rendering/Shaders.hpp"
#include "../Rendering/Textures.hpp"
#include "../World/World.hpp"
#include "Player.hpp"

#include <utility>

class Assets;
class Window;
class FramebufferStack;

class Scene {
	Window& window;
	Assets& assets;
	Ref<Persistence> persistence;
	Ref<World> world;

	// Skybox intégré
	struct Skybox {
		glm::mat4 transform{1};
		VertexArray vertexArray{
			std::vector<glm::vec3>{
				{1, 1, 1},
				{1, 1, -1},
				{1, -1, 1},
				{1, -1, -1},
				{-1, 1, 1},
				{-1, 1, -1},
				{-1, -1, 1},
				{-1, -1, -1},
			},
			std::vector<VertexAttribute>{{3, VertexAttribute::Float, 0}},
			std::vector<uint8_t>{0, 1, 5, 4, 0, 5, 2, 1, 0, 1, 2, 3, 7, 4, 5, 4, 7, 6,
								 1, 3, 7, 1, 7, 5, 4, 6, 0, 2, 0, 6, 3, 2, 6, 7, 3, 6},
		};
		Ref<const Texture> cubeMap;
		Ref<const ShaderProgram> shader;
		float rotation = 0;
		float rotationSpeed = 0.01;

		Skybox(Assets& assets);
		float getRotationSpeed() const { return rotationSpeed; }
		void setRotationSpeed(float speed) { rotationSpeed = speed; }
		void update(const glm::mat4& projection, const glm::mat4& cameraView, float deltaTime);
		void render();
	} skybox;

	Player player;

	const float zNear = 0.1f;
	const float zFar = 1000.0f;
	glm::mat4 projectionMatrix = glm::mat4(1);
	float deltaTime = 1.0f;

	// BlockOutline intégré
	struct BlockOutline {
		Ref<const ShaderProgram> outlinedBlockShader;
		Ref<const CubeMesh> blockMesh;

		BlockOutline(Ref<const CubeMesh> blockMesh, Assets& assets);
		void render(const glm::mat4& transform) const;
	} outline;

	std::vector<Ref<PostProcessEffect>> postProcessingEffects;

	bool isMenuOpen = false;
	bool showIntermediateTextures = false;

	void toggleMenu();
	void updateMouse();

   public:
	Scene(Window& window, Assets& assets, const std::string& savePath);

	void update(float deltaTime);

	void render();
	void renderGui();
	void renderMenu();
	void renderIntermediateTextures();

	void onResized(int32_t width, int32_t height);
	void onKeyEvent(int32_t key, int32_t scancode, int32_t action, int32_t mode);
	void onCursorPositionEvent(double x, double y);
	void onMouseButtonEvent(int32_t button, int32_t action, int32_t mods);
};