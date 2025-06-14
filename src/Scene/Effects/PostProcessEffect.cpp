#include "PostProcessEffect.hpp"

#include "../../Core/Context.hpp"
#include "../../Application/Window.hpp"
#include "../../Rendering/ColorRenderPass.hpp"

PostProcessEffect::PostProcessEffect(Context& context, const Ref<const ShaderProgram> &shader, bool enabled)
	: context(context),
	  shader(shader),
	  enabled(enabled) {}

void PostProcessEffect::render()
{
	if (!enabled) return;
	
	Window& window = context.getWindow();
	int32_t width = window.getWindowWidth();
	int32_t height = window.getWindowHeight();
	
	if (framebuffer == nullptr || framebuffer->getWidth() != width || framebuffer->getHeight() != height)
	{
		framebuffer = std::make_shared<Framebuffer>(width, height, false, 1);
	}

	Ref<FramebufferStack> framebufferStack = window.getFramebufferStack();
	Ref<Framebuffer> colorSource = framebufferStack->peek();
	framebufferStack->push(framebuffer);

	// Bind shader before calling update so uniforms can be set
	shader->bind();
	update();
	ColorRenderPass::renderTextureWithEffect(colorSource->getColorAttachment(0), shader);

	Ref<Framebuffer> resultFbo = framebufferStack->pop();
	ColorRenderPass::renderTexture(resultFbo->getColorAttachment(0), context.getAssetManager());
}
