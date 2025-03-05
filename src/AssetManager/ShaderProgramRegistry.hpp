#pragma once

#include "AssetRegistry.hpp"

class ShaderProgramRegistry : public AssetRegistry<ShaderProgram>
{
	Ref<const ShaderProgram> loadAsset(const std::string &name) override { return std::make_shared<ShaderProgram>(name); }
};