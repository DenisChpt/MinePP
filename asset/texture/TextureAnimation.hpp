#ifndef TEXTURE_ANIMATION_HPP
#define TEXTURE_ANIMATION_HPP

#include <string>
#include <vector>

/**
 * Structure décrivant une animation de texture.
 * - frametime : temps d'affichage (en ticks) pour chaque frame par défaut.
 * - frames : liste des indices de frames dans l'ordre d'animation.
 */
struct TextureAnimation {
	int frametime;
	std::vector<int> frames;
};

/**
 * Charge une animation de texture depuis un fichier .mcmeta.
 * @param mcmetaPath : chemin vers le fichier .mcmeta associé à la texture.
 * @param animation : (out) structure contenant les données d'animation.
 * @return true si le chargement et le parsing ont réussi.
 */
bool loadTextureAnimation(const std::string& mcmetaPath, TextureAnimation& animation);

#endif // TEXTURE_ANIMATION_HPP
