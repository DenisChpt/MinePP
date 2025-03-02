#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

#include <GLFW/glfw3.h>
#include "Config.hpp"

namespace WindowManager {

// Types de fonctions callback
typedef void (*KeyCallbackFunc)(GLFWwindow* window, int key, int scancode, int action, int mods);
typedef void (*CharCallbackFunc)(GLFWwindow* window, unsigned int codepoint);
typedef void (*MouseButtonCallbackFunc)(GLFWwindow* window, int button, int action, int mods);
typedef void (*ScrollCallbackFunc)(GLFWwindow* window, double xoffset, double yoffset);

// Fonctions pour enregistrer des callbacks utilisateur
void setKeyCallback(KeyCallbackFunc callback);
void setCharCallback(CharCallbackFunc callback);
void setMouseButtonCallback(MouseButtonCallbackFunc callback);
void setScrollCallback(ScrollCallbackFunc callback);

// Crée une fenêtre GLFW selon la configuration fournie.
// Retourne un pointeur vers la fenêtre ou nullptr en cas d’erreur.
GLFWwindow* createWindow(const Config &config);

// Installe les callbacks internes sur la fenêtre donnée.
// Ces callbacks internes délèguent ensuite aux callbacks utilisateurs enregistrés.
void setupCallbacks(GLFWwindow *window);

} // namespace WindowManager

#endif // WINDOW_MANAGER_HPP
