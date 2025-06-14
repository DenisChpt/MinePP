/**
 * @file Framebuffers.hpp
 * @brief Gestion unifiée des framebuffers et de leur pile pour le rendu multi-passes
 * 
 * @details Fusionne les classes Framebuffer et FramebufferStack en un seul fichier
 *          pour simplifier la gestion des framebuffers OpenGL
 */

#pragma once

#include "../Common.hpp"
#include "Textures.hpp"

/**
 * @class Framebuffer
 * @brief Encapsule la création et la gestion d'un framebuffer OpenGL
 */
class Framebuffer
{
	uint32_t id = 0;

	std::vector<Ref<Texture>> attachments;
	std::vector<uint32_t> attachmentNames;
	Ref<Texture> depthAttachment = nullptr;

	int32_t width;
	int32_t height;

public:
	Framebuffer(int32_t width, int32_t height, bool createDepthAttachment, int32_t colorAttachmentCount);
	~Framebuffer();

	[[nodiscard]] int32_t getWidth() const { return width; };
	[[nodiscard]] int32_t getHeight() const { return height; };
	[[nodiscard]] bool isValid() const { return id != 0; };
	[[nodiscard]] size_t getAttachmentCount() const { return attachments.size(); };

	Ref<Texture> getDepthAttachment();
	Ref<Texture> getColorAttachment(int32_t index);

	void clearColorAttachment(int32_t index, const glm::vec4 &value);

	void bind(bool forDrawing = true);
	void unbind();

	Framebuffer(const Framebuffer &) = delete;
	Framebuffer(Framebuffer &) = delete;
	Framebuffer(Framebuffer &&) noexcept = delete;
	Framebuffer &operator=(Framebuffer &) = delete;
	Framebuffer &operator=(Framebuffer &&) noexcept = delete;
};

/**
 * @class FramebufferStack
 * @brief Gère une pile de framebuffers pour le rendu multi-passes
 */
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