#ifndef BLOCK_MODEL_HPP
#define BLOCK_MODEL_HPP

#include <string>
#include <vector>
#include <unordered_map>

// Pour la gestion des vecteurs (positions "from" et "to")
// Vous pouvez utiliser glm (par exemple, glm::vec3) ou définir votre propre structure.
#include <glm/glm.hpp>

namespace blockmodel {

/**
 * Structure représentant une face d'un élément du modèle.
 * - texture : référence à la texture (ex : "#texture")
 * - cullface : face pour le culling (optionnel)
 * - uv : coordonnées UV personnalisées (optionnel, 4 valeurs)
 */
struct Face {
	std::string texture;
	std::string cullface;
	std::vector<float> uv;
};

/**
 * Structure représentant un élément (cube) du modèle.
 * - from : coin inférieur gauche (min) dans l'espace du bloc.
 * - to : coin supérieur droit (max).
 * - faces : ensemble de faces (north, south, east, west, up, down).
 */
struct ParsedBlockElement {
	glm::vec3 from;
	glm::vec3 to;
	std::unordered_map<std::string, Face> faces;
};

/**
 * Structure représentant le modèle complet d'un bloc.
 * - parent : référence au modèle parent (pour l'héritage)
 * - textures : mapping des variables de textures vers les références (ex : "particle": "block/acacia_button")
 * - elements : liste des éléments qui composent le modèle.
 */
struct ParsedBlockModel {
	std::string parent;
	std::unordered_map<std::string, std::string> textures;
	std::vector<ParsedBlockElement> elements;
};

} // namespace blockmodel

#endif // BLOCK_MODEL_HPP
