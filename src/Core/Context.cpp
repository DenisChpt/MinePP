#include "Context.hpp"
#include "../Application/Window.hpp"
#include "Assets.hpp"

Context::Context() {
    // Ordre important !
    assets = std::make_unique<Assets>();
    window = std::make_unique<Window>();
    window->setAssets(assets.get());
    assets->loadTextureAtlas();
}

Context::~Context() = default;