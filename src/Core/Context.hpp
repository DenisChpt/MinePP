#pragma once
#include "../Common.hpp"

class Window;
class Assets;

class Context {
private:
    Scoped<Window> window;
    Scoped<Assets> assets;
    
public:
    Context();
    ~Context();
    
    Window& getWindow() { return *window; }
    Assets& getAssets() { return *assets; }
    
    const Window& getWindow() const { return *window; }
    const Assets& getAssets() const { return *assets; }
    
    // Compatibility methods during transition
    Assets& getAssetManager() { return *assets; }
    const Assets& getAssetManager() const { return *assets; }
};