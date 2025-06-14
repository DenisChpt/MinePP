#pragma once

#include "../../AssetManager/AssetManager.hpp"
#include "../../Rendering/FrameBuffer.hpp"
#include "../../Rendering/ShaderProgram.hpp"
#include "../../MinePP.hpp"

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
