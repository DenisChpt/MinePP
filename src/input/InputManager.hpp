#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <GLFW/glfw3.h>
#include "Config.hpp"

// Structure minimale d'état du joueur (position, rotation et temps de mise à jour)
// Dans un projet complet, ce type sera défini dans un module commun (ex. PlayerManager).
struct State {
	float x, y, z;    // Position
	float rx, ry;     // Rotation (en radians)
	float t;          // Temps de dernière mise à jour (pour interpolation, etc.)
};

namespace InputManager {

// --- Initialisation et mise à jour continue ---
/// Initialise la position de la souris.
/// À appeler une fois la fenêtre créée afin d’éviter des sauts lors du premier mouvement.
void initMousePosition(GLFWwindow* window);

/// Met à jour la rotation de la caméra en fonction du déplacement de la souris.
/// Doit être appelée dans la boucle principale pour actualiser l'état de la caméra.
/// - window : fenêtre GLFW
/// - state  : état du joueur (sera modifié)
/// - config : instance de configuration pour récupérer, par exemple, l’option d’inversion de la souris.
void updateCamera(GLFWwindow* window, State &state, const Config &config);

/// Met à jour le déplacement du joueur en fonction des touches pressées.
/// - window : fenêtre GLFW
/// - state  : état du joueur (sera modifié)
/// - flying : indique si le mode vol est activé
/// - config : configuration (pour récupérer les touches de mouvement, de saut, etc.)
/// - dt     : temps écoulé depuis la dernière mise à jour (en secondes)
void updateMovement(GLFWwindow* window, State &state, bool flying, const Config &config, double dt);

// --- Callbacks pour les événements d’entrée ---
/// Ces fonctions sont destinées à être enregistrées via WindowManager.
/// Elles déclenchent (ou délèguent) les actions liées aux événements d’entrée.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void charCallback(GLFWwindow* window, unsigned int codepoint);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

} // namespace InputManager

#endif // INPUT_MANAGER_HPP
