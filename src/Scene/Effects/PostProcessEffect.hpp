#pragma once

#include "../../Core/Assets.hpp"
#include "../../Rendering/Framebuffers.hpp"
#include "../../Rendering/Shaders.hpp"
#include "../../Common.hpp"

class Context;

class PostProcessEffect
{
protected:
	Context& context;
	bool enabled;
	Ref<const ShaderProgram> shader;
	Ref<Framebuffer> framebuffer;

public:
	explicit PostProcessEffect(Context& context, const Ref<const ShaderProgram> &shader, bool enabled = false);

	Ref<const ShaderProgram> getShader() { return shader; };

	virtual void update() = 0;
	virtual void renderGui() = 0;
	void render();

	virtual ~PostProcessEffect() = default;
};
