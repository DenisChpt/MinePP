// Effects.cpp - Implémentation de tous les effets post-process
#include "Effects.hpp"

#include "../Rendering/ColorRenderPass.hpp"

// PostProcessEffect implementation
PostProcessEffect::PostProcessEffect(Window& window, Assets& assets, const Ref<const ShaderProgram> &shader, bool enabled)
	: window(window),
	  assets(assets),
	  shader(shader),
	  enabled(enabled) {}

void PostProcessEffect::render()
{
	if (!enabled) return;
	
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
	ColorRenderPass::renderTexture(resultFbo->getColorAttachment(0), assets);
}

// ChromaticAberrationEffect implementation
ChromaticAberrationEffect::ChromaticAberrationEffect(Window& window, Assets& assets, bool enabled)
	: PostProcessEffect(window, assets, assets.loadShaderProgram("assets/shaders/chromatic_aberration_effect"), enabled) {}

void ChromaticAberrationEffect::renderGui()
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

void ChromaticAberrationEffect::update()
{
	shader->setFloat("start", aberrationStart);
	shader->setFloat("rOffset", aberrationROffset);
	shader->setFloat("gOffset", aberrationGOffset);
	shader->setFloat("bOffset", aberrationBOffset);
}

// CrosshairEffect implementation
CrosshairEffect::CrosshairEffect(Window& window, Assets& assets, bool enabled)
	: PostProcessEffect(window, assets, assets.loadShaderProgram("assets/shaders/crosshair"), enabled) {}

void CrosshairEffect::renderGui()
{
	ImGui::Checkbox("Enable crosshair", &enabled);
	if (enabled)
	{
		ImGui::SliderFloat("Crosshair size", &crosshairSize, 0.01, 1);
		ImGui::SliderFloat("Crosshair vertical width", &crosshairVerticalWidth, 0.01, 1);
		ImGui::SliderFloat("Crosshair horizontal width", &crosshairHorizontalWidth, 0.01, 1);
	}
}

void CrosshairEffect::update()
{
	auto width = window.getWindowWidth();
	auto height = window.getWindowHeight();
	float aspectRatio = width == 0 || height == 0 ? 0 : static_cast<float>(width) / static_cast<float>(height);

	shader->setFloat("size", crosshairSize);
	shader->setFloat("verticalWidth", crosshairVerticalWidth);
	shader->setFloat("horizontalWidth", crosshairHorizontalWidth);
	shader->setFloat("aspectRatio", aspectRatio);
}

// GammaCorrectionEffect implementation
GammaCorrectionEffect::GammaCorrectionEffect(Window& window, Assets& assets, bool enabled)
	: PostProcessEffect(window, assets, assets.loadShaderProgram("assets/shaders/gamma_correction"), enabled) {}

void GammaCorrectionEffect::update() 
{ 
	shader->setFloat("power", power); 
}

void GammaCorrectionEffect::renderGui()
{
	ImGui::Checkbox("Enable gamma correction", &enabled);
	if (enabled)
	{
		ImGui::SliderFloat("Gamma correction power", &power, 0.5f, 3.0f);
	}
}

// GaussianBlurEffect implementation
std::string GaussianBlurEffect::GaussianBlurShader::emitVertexShaderSource() const
{
	return "#version 450 core\n"
		   "layout(location = 0) in vec3 position;\n"
		   "void main() {\n"
		   "    gl_Position = vec4(position, 1);\n"
		   "}";
}

std::string GaussianBlurEffect::GaussianBlurShader::emitFragmentShaderSource() const
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

Ref<const ShaderProgram> GaussianBlurEffect::getBlurShader(int32_t blurStDev)
{
	if (!shaders.contains(blurStDev))
	{
		shaders[blurStDev] = GaussianBlurShader(blurStDev).getShader();
	}
	return shaders.at(blurStDev);
}

GaussianBlurEffect::GaussianBlurEffect(Window& window, Assets& assets, bool enabled) 
	: PostProcessEffect(window, assets, nullptr, enabled) {}

void GaussianBlurEffect::update() 
{ 
	shader = getBlurShader(stDev); 
}

void GaussianBlurEffect::renderGui()
{
	ImGui::Checkbox("Enable gaussian blur effect", &enabled);
	if (enabled)
	{
		ImGui::SliderInt("Gaussian Blur StDev: ", &stDev, 0, 5);
	}
}

// InvertEffect implementation
InvertEffect::InvertEffect(Window& window, Assets& assets, bool enabled)
	: PostProcessEffect(window, assets, assets.loadShaderProgram("assets/shaders/invert_effect"), enabled) {}

void InvertEffect::update() {}

void InvertEffect::renderGui()
{
	ImGui::Checkbox("Enable invert effect", &enabled);
}

// VignetteEffect implementation
VignetteEffect::VignetteEffect(Window& window, Assets& assets, bool enabled)
	: PostProcessEffect(window, assets, assets.loadShaderProgram("assets/shaders/vignette_effect"), enabled) {}

void VignetteEffect::update()
{
	shader->setFloat("intensity", vignetteIntensity);
	shader->setFloat("start", vignetteStart);
}

void VignetteEffect::renderGui()
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