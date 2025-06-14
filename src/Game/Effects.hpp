// Effects.hpp - Consolidation de tous les effets post-process
#pragma once

#include "../Core/Assets.hpp"
#include "../Rendering/Framebuffers.hpp"
#include "../Rendering/Shaders.hpp"
#include "../Application/Window.hpp"
#include "../Common.hpp"

class Context;

// Classe de base PostProcessEffect
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

// ChromaticAberrationEffect
class ChromaticAberrationEffect : public PostProcessEffect
{
	float aberrationStart = 1.5f;
	float aberrationROffset = 0.005;
	float aberrationGOffset = 0.01;
	float aberrationBOffset = -0.005;

public:
	ChromaticAberrationEffect(Context& context, bool enabled);

	void renderGui() override;
	void update() override;
};

// CrosshairEffect
class CrosshairEffect : public PostProcessEffect
{
	float crosshairSize = 0.015f;
	float crosshairVerticalWidth = 0.2f;
	float crosshairHorizontalWidth = 0.15f;

public:
	CrosshairEffect(Context& context, bool enabled);

	void renderGui() override;
	void update() override;
};

// GammaCorrectionEffect
class GammaCorrectionEffect : public PostProcessEffect
{
	float power = 0.85;

public:
	GammaCorrectionEffect(Context& context, bool enabled);

	void update() override;
	void renderGui() override;
};

// GaussianBlurEffect
class GaussianBlurEffect : public PostProcessEffect
{
private:
	int32_t stDev = 2;
	std::map<int32_t, Ref<const ShaderProgram>> shaders;

	class GaussianBlurShader : public ProceduralShader
	{
		int32_t stDev;

	protected:
		std::string emitVertexShaderSource() const override;
		std::string emitFragmentShaderSource() const override;

	public:
		GaussianBlurShader(int32_t stDev) : stDev(stDev) { assert(stDev >= 0 && stDev <= 5); };
	};

	Ref<const ShaderProgram> getBlurShader(int32_t blurStDev);

public:
	GaussianBlurEffect(Context& context, bool enabled);

	void update() override;
	void renderGui() override;
};

// InvertEffect
class InvertEffect : public PostProcessEffect
{
public:
	InvertEffect(Context& context, bool enabled);

	void update() override;
	void renderGui() override;
};

// VignetteEffect
class VignetteEffect : public PostProcessEffect
{
	float vignetteIntensity = 2.9;
	float vignetteStart = 2;

public:
	VignetteEffect(Context& context, bool enabled);

	void update() override;
	void renderGui() override;
};