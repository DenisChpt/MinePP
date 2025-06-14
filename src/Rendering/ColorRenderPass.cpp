#include "ColorRenderPass.hpp"

#include "../Core/Assets.hpp"
#include "../Application/Application.hpp"
#include "../Utils/Utils.hpp"

ColorRenderPass::ColorRenderPass(const Ref<const ShaderProgram> &shader) : shader(shader)
{
	assert(shader != nullptr);
}

void ColorRenderPass::setTexture(const std::string &attachmentName, const Ref<Texture> &texture, int32_t slot)
{
	// Store texture info to be set after binding
	textureBindings.push_back({attachmentName, texture, slot});
}

void ColorRenderPass::render()
{
	TRACE_FUNCTION();
	shader->bind();
	
	// Apply texture bindings after shader is bound
	for (const auto& binding : textureBindings) {
		shader->setTexture(binding.name, binding.texture, binding.slot);
	}
	
	glDisable(GL_DEPTH_TEST);
	FullscreenQuad::getVertexArray()->renderIndexed();
	glEnable(GL_DEPTH_TEST);
}

void ColorRenderPass::renderTextureWithEffect(const Ref<Texture> &texture, const Ref<const ShaderProgram> &effect)
{
	TRACE_FUNCTION();
	
	// Note: The shader should already be bound and configured by the caller
	// We just need to set the texture and render
	ColorRenderPass renderPass(effect);
	renderPass.setTexture("colorTexture", texture, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	renderPass.render();
}

void ColorRenderPass::renderTexture(const Ref<Texture> &texture, Assets& assets)
{
	TRACE_FUNCTION();
	static Ref<const ShaderProgram> colorIdentity = nullptr;
	if (!colorIdentity) {
		colorIdentity = assets.loadShaderProgram("assets/shaders/identity");
	}
	renderTextureWithEffect(texture, colorIdentity);
}
