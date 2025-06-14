#pragma once
#include "../Common.hpp"

class Window;
class AssetManager;
class TextureAtlas;

class Context {
private:
    Scoped<Window> window;
    Scoped<AssetManager> assetManager;
    Scoped<TextureAtlas> textureAtlas;
    
public:
    Context();
    ~Context();
    
    Window& getWindow() { return *window; }
    AssetManager& getAssetManager() { return *assetManager; }
    TextureAtlas& getTextureAtlas() { return *textureAtlas; }
    
    const Window& getWindow() const { return *window; }
    const AssetManager& getAssetManager() const { return *assetManager; }
    const TextureAtlas& getTextureAtlas() const { return *textureAtlas; }
};