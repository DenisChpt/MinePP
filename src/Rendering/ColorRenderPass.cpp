#include "ColorRenderPass.hpp"

#include "../AssetManager/AssetManager.hpp"
#include "../Application/Application.hpp"
#include "../Utils/Utils.hpp"

ColorRenderPass::ColorRenderPass(const Ref<const ShaderProgram> &shader) : shader(shader)
{
	assert(shader != nullptr);
}

void ColorRenderPass::setTexture(const std::string &attachmentName, const Ref<Texture> &texture, int32_t slot)
{
	shader->setTexture(attachmentName, texture, slot);
}

void ColorRenderPass::render()
{
	TRACE_FUNCTION();
	shader->bind();
	glDisable(GL_DEPTH_TEST);
	FullscreenQuad::getVertexArray()->renderIndexed();
	glEnable(GL_DEPTH_TEST);
}

void ColorRenderPass::renderTextureWithEffect(const Ref<Texture> &texture, const Ref<const ShaderProgram> &effect)
{
	TRACE_FUNCTION();
	ColorRenderPass renderPass(effect);

	effect->bind();
	// TODO: Fix this - need window reference
	effect->setInt("screenHeight", 900); // temporary hardcoded
	renderPass.setTexture("colorTexture", texture, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	renderPass.render();
}

void ColorRenderPass::renderTexture(const Ref<Texture> &texture, AssetManager& assetManager)
{
	TRACE_FUNCTION();
	static Ref<const ShaderProgram> colorIdentity = nullptr;
	if (!colorIdentity) {
		colorIdentity = assetManager.loadShaderProgram("assets/shaders/identity");
	}
	renderTextureWithEffect(texture, colorIdentity);
}
