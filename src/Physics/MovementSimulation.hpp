/**
 * @namespace MovementSimulation
 * @brief Fournit une fonction pour simuler le déplacement du joueur en tenant compte des
 * collisions.
 *
 * @details La fonction canMove() vérifie, pour un mouvement donné du joueur (départ -> arrivée),
 *          si la boîte englobante du joueur intersecte des blocs solides ou opaques du monde. Elle
 * utilise le ray casting pour déterminer les blocs susceptibles d'entrer en collision.
 *
 * @param from Position de départ.
 * @param to Position d'arrivée souhaitée.
 * @param world Référence au monde pour vérifier les collisions.
 *
 * @return true si le mouvement est possible, false sinon.
 */

#pragma once

#include "../Common.hpp"
#include "../World/World.hpp"

namespace MovementSimulation {
bool canMove(const glm::vec3& from, const glm::vec3& to, World& world);
};
