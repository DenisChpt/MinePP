// main.cpp
#include <iostream>
#include <chrono>
#include <vector>

// Modules refactorisés
#include "Config.hpp"
#include "WindowManager.hpp"
#include "RenderManager.hpp"
#include "InputManager.hpp"
#include "WorldManager.hpp"
#include "WorkerManager.hpp"
#include "PlayerManager.hpp"
#include "CommandManager.hpp"
#include "NetworkManager.hpp"
#include "DatabaseManager.hpp"
#include "ResourceManager.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Pour simplifier, nous utilisons l'espace de noms std::chrono pour le temps
using Clock = std::chrono::high_resolution_clock;

int main(int argc, char **argv)
{
    // --- Initialisation des systèmes externes ---
    // Initialisation de GLFW
    if (!glfwInit()) {
        std::cerr << "[Main] Erreur d'initialisation de GLFW." << std::endl;
        return -1;
    }
    
    // Création d'une instance de configuration
    Config config;
    
    // Création de la fenêtre via WindowManager
    GLFWwindow* window = WindowManager::createWindow(config);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialisation de GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "[Main] Erreur d'initialisation de GLEW: " 
                  << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Configuration des callbacks de la fenêtre
    WindowManager::setupCallbacks(window);
    // Enregistrement des callbacks du module InputManager
    WindowManager::setKeyCallback(InputManager::keyCallback);
    WindowManager::setCharCallback(InputManager::charCallback);
    WindowManager::setMouseButtonCallback(InputManager::mouseButtonCallback);
    WindowManager::setScrollCallback(InputManager::scrollCallback);
    
    // Récupérer la taille de la fenêtre
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Initialisation du rendu OpenGL via RenderManager
    RenderManager::initGL(config, fbWidth, fbHeight);
    
    // Initialisation du ResourceManager (les ressources seront chargées à la demande)
    // Par exemple, pour charger une texture de base :
    GLuint textureBlock = ResourceManager::loadTexture("textures/texture.png");
    // D'autres ressources (shaders, programmes, etc.) seront chargées par RenderManager ou autres modules.
    
    // Initialisation de la base de données
    if (!DatabaseManager::init(config.getDbPath())) {
        std::cerr << "[Main] Erreur d'initialisation de la base de données." << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Initialisation du réseau
    // On peut utiliser les arguments de ligne de commande pour définir l'adresse et le port, sinon valeurs par défaut.
    std::string serverAddr = "localhost";
    int serverPort = 4080;
    if (argc >= 2) {
        serverAddr = argv[1];
    }
    if (argc >= 3) {
        serverPort = std::atoi(argv[2]);
    }
    if (!NetworkManager::init(serverAddr, serverPort)) {
        std::cerr << "[Main] Erreur de connexion réseau." << std::endl;
    } else {
        NetworkManager::start();
    }
    
    // Initialisation des workers (par exemple, 4 workers)
    WorkerManager workerManager(4);
    workerManager.start();
    
    // Initialisation des joueurs et du monde
    std::vector<Player> players;
    // Création du joueur local (ID 0)
    PlayerState initialState = {0.0f, 64.0f, 0.0f, 0.0f, 0.0f, static_cast<float>(Clock::now().time_since_epoch().count())};
    PlayerManager::createPlayer(players, 0, "LocalPlayer", initialState);
    
    // Initialisation des chunks (vecteur de chunks géré par WorldManager)
    std::vector<Chunk> chunks;
    
    // Initialisation de la position initiale de la souris
    InputManager::initMousePosition(window);
    
    // Variables de temps pour la boucle principale
    auto previousTime = Clock::now();
    
    // Boucle principale
    while (!glfwWindowShouldClose(window)) {
        // Calcul du delta time
        auto currentTime = Clock::now();
        float dt = std::chrono::duration<float>(currentTime - previousTime).count();
        previousTime = currentTime;
        
        // Mise à jour des entrées utilisateur
        InputManager::updateCamera(window, players[0].current, config);
        InputManager::updateMovement(window, players[0].current, /*flying*/ false, config, dt);
        
        // Mise à jour du réseau
        NetworkManager::update(players);
        
        // Mise à jour des workers pour la génération et la régénération des chunks
        workerManager.update(&players[0].current, chunks, config, nullptr); // On passe nullptr pour DatabaseManager si géré globalement
        
        // Mise à jour de l'interpolation des joueurs
        float timeNow = static_cast<float>(Clock::now().time_since_epoch().count());
        for (auto &player : players) {
            // On interpole l'état de chaque joueur pour un rendu fluide.
            PlayerManager::interpolatePlayer(player, timeNow);
        }
        
        // Rendu : on efface l'écran et on configure le viewport
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        
        // Exemple d'attributs shader pour le rendu (ces valeurs devraient être initialisées par RenderManager)
        Attrib blockAttrib, skyAttrib, textAttrib, lineAttrib;
        // Pour cet exemple, on suppose que les programmes shaders ont été chargés ailleurs
        // et que blockAttrib, skyAttrib, etc. contiennent les IDs des programmes et attributs nécessaires.
        // Ici, nous nous concentrons sur l'intégration des modules.
        
        // Rendu du ciel
        GLuint skyBuffer = RenderManager::genSkyBuffer();
        RenderManager::renderSky(skyAttrib, players[0].current, skyBuffer, fbWidth, fbHeight, config.getRenderChunkRadius(), config.getFov(), config.getOrtho());
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // Rendu des chunks du monde
        int totalFaces = RenderManager::renderChunks(blockAttrib, players[0].current, chunks, static_cast<int>(chunks.size()), config.getRenderChunkRadius(), fbWidth, fbHeight, config.getFov(), config.getOrtho());
        
        // Rendu des joueurs
        RenderManager::renderPlayers(blockAttrib, players[0].current, players, static_cast<int>(players.size()), fbWidth, fbHeight, config.getFov(), config.getOrtho());
        
        // Rendu du crosshair et de l'item sélectionné
        RenderManager::renderCrosshairs(lineAttrib, fbWidth, fbHeight, config.getScale());
        RenderManager::renderItem(blockAttrib, fbWidth, fbHeight, config.getScale(), config.getDefaultItemIndex());
        
        // Rendu du texte d'information (par exemple, FPS, position, etc.)
        std::string infoText = "(Exemple d'info)";
        RenderManager::renderText(textAttrib, 0, 12, fbHeight - 24, 12, infoText.c_str(), fbWidth, fbHeight);
        
        // Échange des buffers et gestion des événements
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Fin de la boucle : sauvegarde et nettoyage
    DatabaseManager::commit();
    DatabaseManager::close();
    NetworkManager::stop();
    workerManager.stop();
    ResourceManager::clear();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "[Main] Fermeture de l'application." << std::endl;
    return 0;
}
