/**
 * @class CubeMesh
 * @brief Génère un maillage de cube utilisé pour représenter les blocs ou pour l'affichage d'effets (ex. contours de bloc).
 *
 * @details La classe CubeMesh construit un VertexArray à partir d'un ensemble de BlockVertex pré-définis pour chaque face.
 */


#pragma once

#include "../Rendering/VertexArray.hpp"

class CubeMesh
{
	Ref<VertexArray> cubeVertexArray;

public:
	CubeMesh();
	void render() const;
};
