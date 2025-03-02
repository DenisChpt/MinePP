#ifndef WORLD_HPP
#define WORLD_HPP

#include <functional>
#include "IConfig.hpp"
#include "Noise.hpp" // Pour simplex2, simplex3

namespace World {

// Type de callback qui sera appelé pour chaque bloc généré.
// Les paramètres : coordonnées x, y, z, valeur w, et un pointeur d'argument.
using WorldFunc = std::function<void(int, int, int, int, void*)>;

/**
 * Génère le contenu d’un chunk (zone du monde) identifié par les indices p et q.
 * La génération est basée sur des fonctions de bruit (simplex2/simplex3) et sur les paramètres
 * de configuration passés via l’instance IConfig.
 *
 * @param config L'instance de configuration qui fournit, entre autres, la taille d'un chunk et les options de génération.
 * @param p, q Les indices du chunk.
 * @param func La fonction callback qui sera appelée pour chaque bloc généré.
 * @param arg Un argument opaque passé à la fonction callback.
 */
void createWorld(const IConfig &config, int p, int q, WorldFunc func, void* arg);

} // namespace World

#endif // WORLD_HPP
