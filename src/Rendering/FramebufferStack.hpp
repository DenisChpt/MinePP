/**
 * @class FramebufferStack
 * @brief Gère une pile de framebuffers afin de faciliter le rendu multi-passes et le post-process.
 *
 * @details La classe FramebufferStack permet de pousser et de dépiler des framebuffers,
 *          et de conserver éventuellement des références aux textures intermédiaires pour déboguer les passes de rendu.
 */

#pragma once

#include "../Common.hpp"
#include "FrameBuffer.hpp"

class FramebufferStack
{
	std::vector<Ref<Framebuffer>> stack;
	std::vector<Ref<Texture>> intermediateTextures;
	bool keepIntermediateTextures = false;

public:
	void push(const Ref<Framebuffer> &framebuffer);
	[[nodiscard]] Ref<Framebuffer> peek() const;
	Ref<Framebuffer> pop();

	void setKeepIntermediateTextures(bool keepBuffers);
	void clearIntermediateTextureReferences();
	std::vector<Ref<Texture>> getIntermediateTextures() const;

	[[nodiscard]] bool empty() const;
	[[nodiscard]] size_t size() const;
};
