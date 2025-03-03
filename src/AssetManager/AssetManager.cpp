#include "AssetManager.hpp"

AssetManager *AssetManager::instancePtr = nullptr;

AssetManager::AssetManager() {
  assert(instancePtr == nullptr && "The asset manager is already instantiated");
  instancePtr = this;
};

AssetManager::~AssetManager() {
  instancePtr = nullptr;
};
