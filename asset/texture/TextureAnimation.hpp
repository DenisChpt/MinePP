#ifndef TEXTURE_ANIMATION_HPP
#define TEXTURE_ANIMATION_HPP

#include <string>
#include <vector>

/**
 * @brief Represents texture animation properties read from a .mcmeta file.
 */
struct TextureAnimation {
	int frametime;
	std::vector<int> frames;
};

/**
 * @brief Loads animation data for a texture from a .mcmeta file.
 * @param mcmetaPath The file path to the .mcmeta file.
 * @param animation Output animation data structure.
 * @return True if successfully loaded, otherwise false.
 */
bool loadTextureAnimation(const std::string& mcmetaPath, TextureAnimation& animation);

#endif // TEXTURE_ANIMATION_HPP
