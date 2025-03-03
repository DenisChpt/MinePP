#pragma once

#include "../MinePP.hpp"
#include "FrameBuffer.hpp"

class FramebufferStack {
  std::vector<Ref<Framebuffer>> stack;
  std::vector<Ref<Texture>> intermediateTextures;
  bool keepIntermediateTextures = false;

public:
  void push(const Ref<Framebuffer>& framebuffer);
  [[nodiscard]] Ref<Framebuffer> peek() const;
  Ref<Framebuffer> pop();

  void setKeepIntermediateTextures(bool keepBuffers);
  void clearIntermediateTextureReferences();
  std::vector<Ref<Texture>> getIntermediateTextures() const;

  [[nodiscard]] bool empty() const;
  [[nodiscard]] size_t size() const;
};
