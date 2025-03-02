#include "InputManager.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

// Variables statiques pour mémoriser la dernière position de la souris.
namespace InputManager {

static double lastMouseX = 0.0;
static double lastMouseY = 0.0;
static bool mouseInitialized = false;

void initMousePosition(GLFWwindow* window) {
	glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
	mouseInitialized = true;
}

void updateCamera(GLFWwindow* window, State &state, const Config &config) {
	// Mise à jour uniquement si le curseur est en mode exclusif (caché)
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		float sensitivity = 0.0025f;
		if (mouseInitialized) {
			double deltaX = mx - lastMouseX;
			double deltaY = my - lastMouseY;
			state.rx += static_cast<float>(deltaX) * sensitivity;
			// Inversion verticale en fonction de la configuration
			if (config.isInvertMouse())
				state.ry += static_cast<float>(deltaY) * sensitivity;
			else
				state.ry -= static_cast<float>(deltaY) * sensitivity;

			// Assurer que la rotation horizontale reste dans [0, 2π)
			const float twoPi = 2.0f * 3.14159265358979323846f;
			if (state.rx < 0)
				state.rx += twoPi;
			if (state.rx >= twoPi)
				state.rx -= twoPi;

			// Limiter la rotation verticale à [-π/2, π/2]
			float halfPi = 3.14159265358979323846f / 2.0f;
			state.ry = std::max(state.ry, -halfPi);
			state.ry = std::min(state.ry, halfPi);
		}
		lastMouseX = mx;
		lastMouseY = my;
	} else {
		// Si le curseur n'est pas en mode exclusif, actualiser simplement la position
		glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
		mouseInitialized = true;
	}
}

void updateMovement(GLFWwindow* window, State &state, bool flying, const Config &config, double dt) {
	// Récupération de l'état des touches de mouvement
	int keyForward = glfwGetKey(window, config.getKeyForward());
	int keyBackward = glfwGetKey(window, config.getKeyBackward());
	int keyLeft = glfwGetKey(window, config.getKeyLeft());
	int keyRight = glfwGetKey(window, config.getKeyRight());

	float moveX = 0.0f, moveZ = 0.0f;
	if (keyForward == GLFW_PRESS)
		moveZ -= 1.0f;
	if (keyBackward == GLFW_PRESS)
		moveZ += 1.0f;
	if (keyLeft == GLFW_PRESS)
		moveX -= 1.0f;
	if (keyRight == GLFW_PRESS)
		moveX += 1.0f;

	// Normalisation du vecteur de déplacement
	float len = std::sqrt(moveX * moveX + moveZ * moveZ);
	if (len > 0.0f) {
		moveX /= len;
		moveZ /= len;
	}
	// Calcul de l'angle de déplacement
	float strafeAngle = std::atan2(moveX, moveZ);

	// Déplacement selon la vitesse (différente en mode vol ou marche)
	float speed = flying ? 20.0f : 5.0f;
	float distance = speed * static_cast<float>(dt);

	// La rotation horizontale (rx) définit l’orientation de la caméra
	float sin_rx = std::sin(state.rx);
	float cos_rx = std::cos(state.rx);

	// Calcul de la direction du mouvement dans le repère monde
	float dx = sin_rx * std::sin(strafeAngle) + cos_rx * std::cos(strafeAngle);
	float dz = cos_rx * std::sin(strafeAngle) - sin_rx * std::cos(strafeAngle);

	// Mise à jour de la position
	state.x += dx * distance;
	state.z += dz * distance;

	// Pour le vol, on peut également gérer le mouvement vertical (par exemple avec la touche de saut)
	int keyJump = glfwGetKey(window, config.getKeyJump());
	if (flying && keyJump == GLFW_PRESS)
		state.y += distance;
	// Pour un mode marche, la gestion de la gravité et des collisions sera traitée ailleurs.
}

// --- Callbacks d'évènements ---

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Exemple minimal : afficher l'action sur la touche 'F'
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_F) {
			std::cout << "[InputManager] Touche F pressée : basculement du mode vol (à gérer par PlayerManager)" << std::endl;
		}
		// D'autres actions liées aux touches de commande peuvent être ajoutées ici.
	}
}

void charCallback(GLFWwindow* window, unsigned int codepoint) {
	// Par exemple, afficher le caractère reçu (la gestion complète du texte sera déléguée à CommandManager)
	if (codepoint >= 32 && codepoint < 128) {
		std::cout << "[InputManager] Saisie caractère : " << static_cast<char>(codepoint) << std::endl;
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			std::cout << "[InputManager] Bouton gauche de la souris pressé" << std::endl;
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			std::cout << "[InputManager] Bouton droit de la souris pressé" << std::endl;
		} else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			std::cout << "[InputManager] Bouton central de la souris pressé" << std::endl;
		}
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	std::cout << "[InputManager] Défilement de la souris: xoffset = " << xoffset
			  << ", yoffset = " << yoffset << std::endl;
	// Par exemple, ce callback pourrait être utilisé pour faire défiler la sélection d'items.
}

} // namespace InputManager
