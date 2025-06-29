/**
 * @class ColorRenderPass
 * @brief Effectue une passe de rendu couleur avec application d'effets post-process via un shader.
 *
 * @details La classe ColorRenderPass permet de lier un ShaderProgram, d'y associer une texture (par
 * exemple le rendu d'une frame intermédiaire) et d'exécuter le rendu sur un quad plein écran.
 */

#pragma once

#include "Buffers.hpp"
#include "Shaders.hpp"

class Assets;

class ColorRenderPass {
	Ref<const ShaderProgram> shader;

	struct TextureBinding {
		std::string name;
		Ref<Texture> texture;
		int32_t slot;
	};
	std::vector<TextureBinding> textureBindings;

   public:
	explicit ColorRenderPass(const Ref<const ShaderProgram>& shader);

	void setTexture(const std::string& attachmentName, const Ref<Texture>& texture, int32_t slot);
	void render();

	static void renderTextureWithEffect(const Ref<Texture>& texture,
										const Ref<const ShaderProgram>& effect);
	static void renderTexture(const Ref<Texture>& texture, Assets& assets);
};
