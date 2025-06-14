#include "PostProcessEffect.hpp"

#include "../../Application/Window.hpp"
#include "../../Rendering/ColorRenderPass.hpp"

PostProcessEffect::PostProcessEffect(const Ref<const ShaderProgram> &shader, bool enabled)
	: shader(shader),
	  enabled(enabled) {}

void PostProcessEffect::render()
{
	if (!enabled)
	{
		return;
	}

	// TODO: Need proper Context injection
	// Window &window = Window::instance();
	int32_t width = 1200; // temporary hardcoded
	int32_t height = 900; // temporary hardcoded
	if (framebuffer == nullptr || framebuffer->getWidth() != width || framebuffer->getHeight() != height)
	{
		framebuffer = std::make_shared<Framebuffer>(width, height, false, 1);
	}

	// TODO: Need proper framebuffer stack access
	// Ref<FramebufferStack> framebufferStack = window.getFramebufferStack();
	// For now, skip the effect
	return;
	/*
	Ref<Framebuffer> colorSource = framebufferStack->peek();
	framebufferStack->push(framebuffer);

	update();
	ColorRenderPass::renderTextureWithEffect(colorSource->getColorAttachment(0), shader);

	Ref<Framebuffer> resultFbo = framebufferStack->pop();
	// ColorRenderPass::renderTexture(resultFbo->getColorAttachment(0), assetManager);
	*/
}
