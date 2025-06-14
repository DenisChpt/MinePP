// Scene.cpp refactorisé - mise à jour des includes
#include "Scene.hpp"

#include <Frustum.h>

#include "../Application/Application.hpp"
#include "../Math/Math.hpp"
#include "../Math/Math.inl"
#include "../Utils/Utils.hpp"
#include "../World/BlockTypes.hpp"
#include "Behaviors/ParticleBehaviors.hpp"
#include "../Core/Context.hpp"
#include "../Application/Window.hpp"
#include "../Core/Assets.hpp"

// Implémentation de Skybox
Scene::Skybox::Skybox(Assets& assets)
{
	cubeMap = assets.loadCubeMap(
		"assets/textures/skybox/empty.png;"
		"assets/textures/skybox/empty.png;"
		"assets/textures/skybox/sun.png;"
		"assets/textures/skybox/moon.png;"
		"assets/textures/skybox/empty.png;"
		"assets/textures/skybox/empty.png");
	shader = assets.loadShaderProgram("assets/shaders/skybox");
}

void Scene::Skybox::update(const glm::mat4 &projection, const glm::mat4 &cameraView, float deltaTime)
{
	rotation += rotationSpeed * deltaTime;
	transform = projection * glm::mat4(glm::mat3(cameraView));
}

void Scene::Skybox::render()
{
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	shader->bind();
	shader->setTexture("cubeMap", cubeMap, 1);
	shader->setMat4("transform", transform * glm::rotate(rotation, glm::vec3(1, 0, 0)));
	vertexArray.renderIndexed();

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

// Implémentation de BlockOutline
Scene::BlockOutline::BlockOutline(Ref<const CubeMesh> blockMesh, Assets& assets)
	: outlinedBlockShader(assets.loadShaderProgram("assets/shaders/outline")),
	  blockMesh(std::move(blockMesh)) {}

void Scene::BlockOutline::render(const glm::mat4 &transform) const
{
	outlinedBlockShader->bind();
	outlinedBlockShader->setMat4("MVP", transform);
	blockMesh->render();
}

Scene::Scene(Context& context, const std::string &savePath)
	: context(context),
	  persistence(std::make_shared<Persistence>(savePath)),
	  world(std::make_shared<World>(
		  context,
		  persistence,
		  std::vector{std::static_pointer_cast<WorldBehavior>(std::make_shared<LavaParticleBehavior>(context.getAssetManager())),
					  std::static_pointer_cast<WorldBehavior>(std::make_shared<BlockBreakParticleBehavior>(context.getAssetManager()))},
		  1337)),
	  skybox(context.getAssetManager()),
	  player(world, persistence),
	  outline(std::make_shared<CubeMesh>(), context.getAssetManager())
{
	TRACE_FUNCTION();
	
	// Initialize post-processing effects avec Context
	postProcessingEffects = {
		std::make_shared<CrosshairEffect>(context, true),
		std::make_shared<ChromaticAberrationEffect>(context, false),
		std::make_shared<InvertEffect>(context, false),
		std::make_shared<VignetteEffect>(context, true),
		std::make_shared<GammaCorrectionEffect>(context, true),
		std::make_shared<GaussianBlurEffect>(context, false)
	};
	
	onResized(context.getWindow().getWindowWidth(), context.getWindow().getWindowHeight());
	updateMouse();
}

void Scene::update(float dt)
{
	TRACE_FUNCTION();
	deltaTime = dt;
	player.update(deltaTime);
	world->update(player.getPosition(), deltaTime);
	skybox.update(projectionMatrix, player.getViewMatrix(), deltaTime);
}

void Scene::toggleMenu()
{
	isMenuOpen = !isMenuOpen;
	updateMouse();
}

void Scene::updateMouse()
{
	TRACE_FUNCTION();
	if (isMenuOpen)
	{
		player.resetMousePosition();
		context.getWindow().unlockMouse();
	}
	else
	{
		context.getWindow().lockMouse();
	}
}

void Scene::render()
{
	TRACE_FUNCTION();
	skybox.render();

	const glm::mat4 mvp = projectionMatrix * player.getViewMatrix();
	Frustum frustum(mvp);
	const int32_t width = context.getWindow().getWindowWidth();
	const int32_t height = context.getWindow().getWindowHeight();

	static Ref<Framebuffer> framebuffer = nullptr;
	if (framebuffer == nullptr || framebuffer->getWidth() != width || framebuffer->getHeight() != height)
	{
		framebuffer = std::make_shared<Framebuffer>(width, height, true, 1);
	}

	context.getWindow().getFramebufferStack()->push(framebuffer);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	world->renderOpaque(mvp, player.getPosition(), frustum);
	auto opaqueRender = context.getWindow().getFramebufferStack()->pop();

	world->renderTransparent(mvp, player.getPosition(), frustum, zNear, zFar, opaqueRender);

	if (WorldRayCast ray{player.getPosition(), player.getLookDirection(), *world, Player::Reach})
	{
		outline.render(mvp * glm::translate(ray.getHitTarget().position));
	}

	for (auto &effect : postProcessingEffects)
	{
		effect->render();
	}
}

void Scene::renderMenu()
{
	TRACE_FUNCTION();
	if (ImGui::Begin("Menu"))
	{
		ImGui::Text("Frame Time: %fms", deltaTime * 1000);
		glm::vec3 position = player.getPosition();
		ImGui::Text("Player position: x:%f, y:%f, z:%f", position.x, position.y, position.z);
		glm::vec3 lookDirection = player.getLookDirection();
		ImGui::Text("Player direction: x:%f, y:%f, z:%f", lookDirection.x, lookDirection.y, lookDirection.z);

		ImGui::Spacing();
		ImGui::Spacing();

		bool isSurvival = player.getIsSurvivalMovement();
		if (ImGui::Checkbox("Enable \"physics\"", &isSurvival))
		{
			player.setSurvivalMovement(isSurvival);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Checkbox("Show intermediate textures", &showIntermediateTextures))
		{
			context.getWindow().getFramebufferStack()->setKeepIntermediateTextures(showIntermediateTextures);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		for (auto &effect : postProcessingEffects)
		{
			effect->renderGui();

			ImGui::Spacing();
			ImGui::Spacing();
		}

		BlockData::BlockType blockToPlace = player.getBlockToPlace();
		ImGui::Text("Selected Block: %s", BlockName::blockTypeToName(blockToPlace));

		ImGui::Spacing();
		ImGui::Spacing();

		BlockName::NameArray names = BlockName::getBlockNames();
		int32_t selected = BlockName::blockTypeToIndex(blockToPlace);
		if (ImGui::ListBox("Select a block to place", &selected, &names[0], names.size()))
		{
			player.setBlockToPlace(BlockName::BlockNames[selected].first);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		int32_t useOcclusion = world->getUseAmbientOcclusion() ? 1 : 0;
		if (ImGui::SliderInt("Use ambient occlusion", &useOcclusion, 0, 1))
		{
			world->setUseAmbientOcclusion(useOcclusion == 1);
		}

		ImGui::Spacing();

		int32_t distance = world->getViewDistance();
		if (ImGui::SliderInt("Max render distance", &distance, 1, 64))
		{
			world->setViewDistance(distance);
		}

		ImGui::Spacing();

		float speed = skybox.getRotationSpeed();
		if (ImGui::SliderFloat("Night/Day cycle speed", &speed, 0, 10))
		{
			skybox.setRotationSpeed(speed);
		}

		ImGui::Spacing();

		float movementSpeed = player.getMovementSpeedMultiplier();
		if (ImGui::SliderFloat("Player movement speed multiplier", &movementSpeed, 1.0f, 10.0f))
		{
			player.setMovementSpeedMultiplier(movementSpeed);
		}

		ImGui::Spacing();

		float jumpHeight = player.getJumpHeightMultiplier();
		if (ImGui::SliderFloat("Player jump height multiplier", &jumpHeight, 1.0f, 10.0f))
		{
			player.setJumpHeightMultiplier(jumpHeight);
		}

		ImGui::Spacing();

		float gravity = player.getGravityConstant() / 10;
		if (ImGui::SliderFloat("Gravity", &gravity, -5, 10.0f))
		{
			player.setGravityConstant(gravity * 10);
		}

		ImGui::Spacing();

		if (ImGui::Button("Reset gravity"))
		{
			player.setGravityConstant(Player::DefaultGravity);
		}

		ImGui::Spacing();
		ImGui::Spacing();
		{
			const uint32_t pathLength = 256;
			static char textureAtlasPath[pathLength] = "";
			ImGui::InputText("Custom texture atlas path", textureAtlasPath, pathLength);
			if (ImGui::Button("Load texture atlas"))
			{
				Ref<const Texture> atlas = context.getAssetManager().loadTexture(textureAtlasPath);
				if (atlas != nullptr)
				{
					world->setTextureAtlas(atlas);
				}
			}
		}

		ImGui::Spacing();
		ImGui::Spacing();
		{
			const uint32_t pathLength = 256;
			static char textureAtlasPath[pathLength] = "";
			ImGui::InputText("Save file path", textureAtlasPath, pathLength);
			if (ImGui::Button("Load World"))
			{
				if (std::filesystem::exists(textureAtlasPath))
				{
					// TODO: Implement proper scene switching with context
					// Application::instance().setScene(std::make_shared<Scene>(textureAtlasPath));
				}
			}
		}
	}

	ImGui::End();
}

void Scene::renderIntermediateTextures()
{
	TRACE_FUNCTION();
	if (ImGui::Begin("Intermediate Textures"))
	{
		for (const auto &texture : context.getWindow().getFramebufferStack()->getIntermediateTextures())
		{
			ImGui::Text("%u", texture->getId());
			ImGui::Image(reinterpret_cast<ImTextureID>(texture->getId()), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
		}
	}
	ImGui::End();
}

void Scene::renderGui()
{
	TRACE_FUNCTION();
	if (showIntermediateTextures)
	{
		renderIntermediateTextures();
	}

	if (isMenuOpen)
	{
		renderMenu();
	}
}

void Scene::onResized(int32_t width, int32_t height)
{
	TRACE_FUNCTION();
	float aspectRatio = width == 0 || height == 0 ? 0 : static_cast<float>(width) / static_cast<float>(height);
	projectionMatrix = glm::perspective<float>(glm::radians(70.0f), aspectRatio, zNear, zFar);
}

void Scene::onKeyEvent(int32_t key, int32_t scancode, int32_t action, int32_t mode)
{
	TRACE_FUNCTION();
	if (key == GLFW_KEY_ESCAPE)
	{
		if (action == GLFW_PRESS)
		{
			toggleMenu();
		}
		return;
	}
	if (!isMenuOpen)
	{
		player.onKeyEvent(key, scancode, action, mode);
	}
}

void Scene::onMouseButtonEvent(int32_t button, int32_t action, int32_t mods)
{
	TRACE_FUNCTION();
	if (!isMenuOpen)
	{
		player.onMouseButtonEvent(button, action, mods);
	}
}

void Scene::onCursorPositionEvent(double x, double y)
{
	TRACE_FUNCTION();
	if (!isMenuOpen)
	{
		player.onCursorPositionEvent(x, y);
	}
}
