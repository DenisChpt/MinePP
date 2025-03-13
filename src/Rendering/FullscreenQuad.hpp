/**
 * @namespace FullscreenQuad
 * @brief Fournit un VertexArray représentant un quad couvrant l'intégralité de l'écran.
 *
 * @details La fonction getVertexArray() retourne un quad statique utilisé pour le rendu post-process (appliquer un effet sur la frame).
 */

#pragma once
#include "VertexArray.hpp"

namespace FullscreenQuad
{
	Ref<VertexArray> getVertexArray();
}
