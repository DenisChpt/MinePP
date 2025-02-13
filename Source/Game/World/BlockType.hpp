#pragma once


/**
 * @brief Types de blocs gérés dans le jeu.
 *        On y inclut les blocs de base : Air, Dirt, Grass, Stone, Water, Leaves, Wood, etc.
 */
enum class BlockType {
	Air,
	Dirt,
	Grass,
	Stone,
	Water,
	Leaves,
	Log,
	// éventuellement d’autres...
};

/**
 * @brief Indique si un bloc est considéré comme opaque.
 *        Les blocs opaques masquent les faces adjacentes et n'ont pas besoin d'être rendus derrière.
 */
inline bool isOpaque(BlockType type) {
	switch (type) {
	case BlockType::Air:
	case BlockType::Water:
	case BlockType::Leaves:
		return false;
	default:
		return true;
	}
}

/**
 * @brief Indique si un bloc est totalement transparent (comme l'air) 
 *        ou semi-transparent (eau, feuilles).
 *        Ici, on simplifie : "transparent" = pas opaque (sauf si tu veux gérer Leaves autrement).
 */
inline bool isTransparent(BlockType type) {
	// On considère Water et Leaves comme semi-transparents, Air comme totalement transparent
	// Selon ta logique d'alpha, tu peux affiner.
	switch (type) {
	case BlockType::Air:
	case BlockType::Water:
	case BlockType::Leaves:
		return true;
	default:
		return false;
	}
}
