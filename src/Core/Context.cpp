#include "Context.hpp"
#include "../Application/Window.hpp"
#include "../AssetManager/AssetManager.hpp"
#include "../Application/Gui.hpp"
#include "../AssetManager/TextureAtlas.hpp"

Context::Context() {
    // Ordre important !
    assetManager = std::make_unique<AssetManager>();
    window = std::make_unique<Window>();
    gui = std::make_unique<Gui>(*window);
    textureAtlas = std::make_unique<TextureAtlas>(*assetManager);
}

Context::~Context() = default;