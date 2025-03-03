#pragma once

#include "../Util/Util.hpp"
#include "AssetRegistry.hpp"

class TextRegistry : public AssetRegistry<std::string> {
  Ref<const std::string> loadAsset(const std::string &name) override { return Util::readBinaryFile(name); }
};
