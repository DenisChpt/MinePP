/**
 * @struct Image
 * @brief Structure stockant les données d'une image (largeur, hauteur et tableau de pixels).
 *
 * @details La méthode subImage() permet d'extraire une sous-image à partir d'un offset et d'une taille donnés.
 */

#pragma once

#include "../MinePP.hpp"

struct Image
{
	uint32_t width, height;
	std::vector<uint8_t> data;

	Image subImage(glm::uvec2 offset, glm::uvec2 extent) const;
};