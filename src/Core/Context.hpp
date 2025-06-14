#pragma once
#include "../MinePP.hpp"

class Window;
class AssetManager;
class Gui;
class TextureAtlas;

class Context {
private:
    Scoped<Window> window;
    Scoped<AssetManager> assetManager;
    Scoped<Gui> gui;
    Scoped<TextureAtlas> textureAtlas;
    
public:
    Context();
    ~Context();
    
    Window& getWindow() { return *window; }
    AssetManager& getAssetManager() { return *assetManager; }
    Gui& getGui() { return *gui; }
    TextureAtlas& getTextureAtlas() { return *textureAtlas; }
    
    const Window& getWindow() const { return *window; }
    const AssetManager& getAssetManager() const { return *assetManager; }
    const Gui& getGui() const { return *gui; }
    const TextureAtlas& getTextureAtlas() const { return *textureAtlas; }
};