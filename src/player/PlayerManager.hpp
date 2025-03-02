#ifndef PLAYER_MANAGER_HPP
#define PLAYER_MANAGER_HPP

#include <vector>
#include <string>
#include <GL/glew.h>
#include "InputManager.hpp"  // Pour la structure State

// Structure représentant l'état d'un joueur, utilisée pour l'interpolation.
struct PlayerState {
	float x, y, z;    // Position
	float rx, ry;     // Rotation (en radians)
	float t;          // Temps de la dernière mise à jour
};

// Structure représentant un joueur.
struct Player {
	int id;
	std::string name;
	PlayerState current;   // État actuel (cible)
	PlayerState previous;  // État précédent (pour interpolation)
	GLuint buffer;         // Buffer OpenGL pour le modèle du joueur (mis à jour par RenderManager)
};

namespace PlayerManager {

	/// Crée un joueur et l'ajoute à la liste.
	void createPlayer(std::vector<Player>& players, int id, const std::string &name, const PlayerState &state);

	/// Retourne un pointeur vers le joueur correspondant à l'ID, ou nullptr s'il n'existe pas.
	Player* findPlayer(std::vector<Player>& players, int id);

	/// Met à jour l'état d'un joueur avec un nouvel état.
	/// Si interpolate est vrai, l'état précédent est mis à jour pour permettre l'interpolation.
	void updatePlayer(std::vector<Player>& players, int id, const PlayerState &newState, bool interpolate);

	/// Interpole l'état du joueur en fonction du temps courant.
	/// Cette fonction calcule l'état intermédiaire à utiliser pour le rendu.
	void interpolatePlayer(Player &player, float currentTime);

	/// Supprime un joueur de la liste en fonction de son ID.
	void deletePlayer(std::vector<Player>& players, int id);

} // namespace PlayerManager

#endif // PLAYER_MANAGER_HPP
