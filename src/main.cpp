#include "Application/Application.hpp"
#include "Application/Window.hpp"
#include "Core/Assets.hpp"
#include "Scene/Scene.hpp"

int main()
{
    // Initialize Window
    Window window;
    if (!window.isValid())
    {
        return -1;
    }
    
    // Initialize Assets
    Assets assets;
    
    // Set assets pointer in window (needed for rendering)
    window.setAssets(&assets);
    
    // Create Application
    Application app(window, assets);
    
    // Create Scene with a default save path
    // If the file doesn't exist, a new world will be generated
    auto scene = std::make_shared<Scene>(window, assets, "saves/world");
    app.setScene(scene);
    
    // Run the application
    return app.run();
}