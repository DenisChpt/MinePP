#pragma once

#include "../Rendering/Shader.hpp"
#include "AssetRegistry.hpp"

class ShaderRegistry : public AssetRegistry<Shader> {
  Ref<const Shader> loadAsset(const std::string &name) override { return std::make_shared<Shader>(name); }
};