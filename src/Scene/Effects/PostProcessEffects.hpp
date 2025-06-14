// Nouveau fichier : consolidation de tous les effets post-process
#pragma once

#include "PostProcessEffect.hpp"
#include "../../Application/Window.hpp"			// Ajouté pour CrosshairEffect
#include "../../Rendering/ProceduralShader.hpp" // Ajouté pour GaussianBlurEffect
#include "../../Core/Context.hpp"
#include "../../Core/Assets.hpp"

// ChromaticAberrationEffect
class ChromaticAberrationEffect : public PostProcessEffect
{
	float aberrationStart = 1.5f;
	float aberrationROffset = 0.005;
	float aberrationGOffset = 0.01;
	float aberrationBOffset = -0.005;

public:
	ChromaticAberrationEffect(Context& context, bool enabled)
		: PostProcessEffect(context, context.getAssets().loadShaderProgram("assets/shaders/chromatic_aberration_effect"), enabled) {}

	void renderGui() override
	{
		ImGui::Checkbox("Enable chromatic aberration effect", &enabled);
		if (enabled)
		{
			ImGui::SliderFloat("Aberration start", &aberrationStart, 0.5, 3);
			ImGui::SliderFloat("Aberration R Offset", &aberrationROffset, -0.01, 0.01);
			ImGui::SliderFloat("Aberration G Offset", &aberrationGOffset, -0.01, 0.01);
			ImGui::SliderFloat("Aberration B Offset", &aberrationBOffset, -0.01, 0.01);
		}
	}

	void update() override
	{
		shader->setFloat("start", aberrationStart);
		shader->setFloat("rOffset", aberrationROffset);
		shader->setFloat("gOffset", aberrationGOffset);
		shader->setFloat("bOffset", aberrationBOffset);
	}
};

// CrosshairEffect
class CrosshairEffect : public PostProcessEffect
{
	float crosshairSize = 0.015f;
	float crosshairVerticalWidth = 0.2f;
	float crosshairHorizontalWidth = 0.15f;

public:
	CrosshairEffect(Context& context, bool enabled)
		: PostProcessEffect(context, context.getAssets().loadShaderProgram("assets/shaders/crosshair"), enabled) {}

	void renderGui() override
	{
		ImGui::Checkbox("Enable crosshair", &enabled);
		if (enabled)
		{
			ImGui::SliderFloat("Crosshair size", &crosshairSize, 0.01, 1);
			ImGui::SliderFloat("Crosshair vertical width", &crosshairVerticalWidth, 0.01, 1);
			ImGui::SliderFloat("Crosshair horizontal width", &crosshairHorizontalWidth, 0.01, 1);
		}
	}

	void update() override
	{
		Window& window = context.getWindow();
		auto width = window.getWindowWidth();
		auto height = window.getWindowHeight();
		float aspectRatio = width == 0 || height == 0 ? 0 : static_cast<float>(width) / static_cast<float>(height);

		shader->setFloat("size", crosshairSize);
		shader->setFloat("verticalWidth", crosshairVerticalWidth);
		shader->setFloat("horizontalWidth", crosshairHorizontalWidth);
		shader->setFloat("aspectRatio", aspectRatio);
	}
};

// GammaCorrectionEffect
class GammaCorrectionEffect : public PostProcessEffect
{
	float power = 0.85;

public:
	GammaCorrectionEffect(Context& context, bool enabled)
		: PostProcessEffect(context, context.getAssets().loadShaderProgram("assets/shaders/gamma_correction"), enabled) {};

	void update() override { shader->setFloat("power", power); }

	void renderGui() override
	{
		ImGui::Checkbox("Enable gamma correction", &enabled);
		if (enabled)
		{
			ImGui::SliderFloat("Gamma correction power", &power, 0.5f, 3.0f);
		}
	}
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
		std::string emitVertexShaderSource() const override
		{
			return "#version 450 core\n"
				   "layout(location = 0) in vec3 position;\n"
				   "void main() {\n"
				   "    gl_Position = vec4(position, 1);\n"
				   "}";
		}

		std::string emitFragmentShaderSource() const override
		{
			static std::vector<std::vector<uint32_t>> binomials = {{1},
																   {1, 2, 1},
																   {1, 4, 6, 4, 1},
																   {1, 6, 15, 20, 15, 6, 1},
																   {1, 8, 28, 56, 70, 56, 28, 8, 1},
																   {1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1}};
			static std::vector<float> binomialSums = {1, 16, 256, 4096, 65536, 1048576};
			std::stringstream ss;
			ss << "#version 450 core\n"
				  "uniform sampler2D colorTexture;\n"
				  "layout(location = 0) out vec4 color;\n"
				  "float kernel[] = {\n";
			uint32_t kernelSize = stDev * 2 + 1;

			for (uint32_t i = 0; i < kernelSize; i++)
			{
				ss << "    ";
				for (uint32_t j = 0; j < kernelSize; j++)
				{
					if (i || j)
					{
						ss << ", ";
					}
					ss << (static_cast<float>(binomials[stDev][i] * binomials[stDev][j]) / binomialSums[stDev]);
				}
				ss << '\n';
			}
			ss << "};\n"
				  "void main() {\n"
				  "    vec3 pixel = vec3(0);\n"
				  "    ivec2 center = ivec2(gl_FragCoord.xy);\n"
			   << "    for (int i = -" << stDev << "; i <= " << stDev << "; ++i) {\n"
			   << "        int xKernelIndex = i + " << stDev << ";\n"
			   << "        for (int j = -" << stDev << "; j <= " << stDev << "; ++j) {\n"
			   << "            int yKernelIndex = j + " << stDev << ";\n"
			   << "            ivec2 pixelOffset = center + ivec2(i, j);\n"
			   << "            float kernelValue = kernel[xKernelIndex +" << kernelSize << " * yKernelIndex];\n"
			   << "            pixel += texelFetch(colorTexture, pixelOffset, 0).rgb * kernelValue;\n"
				  "        }\n"
				  "    }\n"
				  "    vec4 centerPixel = texelFetch(colorTexture, center, 0);\n"
				  "    color = vec4(pixel, centerPixel.a);\n"
				  "}";
			return ss.str();
		}

	public:
		GaussianBlurShader(int32_t stDev) : stDev(stDev) { assert(stDev >= 0 && stDev <= 5); };
	};

	Ref<const ShaderProgram> getBlurShader(int32_t blurStDev)
	{
		if (!shaders.contains(blurStDev))
		{
			// Correction : utiliser getShader() de ProceduralShader
			shaders[blurStDev] = GaussianBlurShader(blurStDev).getShader();
		}
		return shaders.at(blurStDev);
	}

public:
	GaussianBlurEffect(Context& context, bool enabled) : PostProcessEffect(context, nullptr, enabled) {};

	void update() override { shader = getBlurShader(stDev); };
	void renderGui() override
	{
		ImGui::Checkbox("Enable gaussian blur effect", &enabled);
		if (enabled)
		{
			ImGui::SliderInt("Gaussian Blur StDev: ", &stDev, 0, 5);
		}
	}
};

// InvertEffect
class InvertEffect : public PostProcessEffect
{
public:
	InvertEffect(Context& context, bool enabled)
		: PostProcessEffect(context, context.getAssets().loadShaderProgram("assets/shaders/invert_effect"), enabled) {}

	void update() override {}

	void renderGui() override
	{
		ImGui::Checkbox("Enable invert effect", &enabled);
	}
};

// VignetteEffect
class VignetteEffect : public PostProcessEffect
{
	float vignetteIntensity = 2.9;
	float vignetteStart = 2;

public:
	VignetteEffect(Context& context, bool enabled)
		: PostProcessEffect(context, context.getAssets().loadShaderProgram("assets/shaders/vignette_effect"), enabled) {}

	void update() override
	{
		shader->setFloat("intensity", vignetteIntensity);
		shader->setFloat("start", vignetteStart);
	}

	void renderGui() override
	{
		ImGui::Checkbox("Enable vignette effect", &enabled);
		if (enabled)
		{
			float invertedIntensity = 4 - vignetteIntensity;
			if (ImGui::SliderFloat("Vignette intensity", &invertedIntensity, 1, 3))
			{
				vignetteIntensity = 4 - invertedIntensity;
			}
			ImGui::SliderFloat("Vignette start", &vignetteStart, 0, 3);
		}
	}
};