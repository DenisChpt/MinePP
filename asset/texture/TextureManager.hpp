#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <glad/glad.h>

/**
 * @brief Represents a rectangular region (in UV space) within a texture atlas.
 */
struct TextureRegion {
	float u;  
	float v;  
	float u2; 
	float v2; 
};

/**
 * @brief Manages loading multiple textures and packing them into a single texture atlas.
 */
class TextureManager {
public:
	TextureManager();
	~TextureManager();

	/**
	 * @brief Loads and assembles a texture atlas from a list of texture file paths.
	 * @param textures A list of (textureName, filePath) pairs.
	 * @return True if successful, otherwise false.
	 */
	bool loadTextureAtlas(const std::vector<std::pair<std::string, std::string>>& textures);

	/**
	 * @brief Gets the OpenGL texture ID for the atlas.
	 */
	unsigned int getAtlasTextureID() const;

	/**
	 * @brief Binds the atlas to the given texture unit.
	 * @param textureUnit The texture unit to bind to (e.g., GL_TEXTURE0).
	 */
	void bindAtlasTexture(GLenum textureUnit = GL_TEXTURE0) const;

	/**
	 * @brief Retrieves UV coordinate information of a texture by name.
	 * @param textureName The name assigned to the texture.
	 * @param region Output parameter for the region.
	 * @return True if the texture name exists, otherwise false.
	 */
	bool getTextureRegion(const std::string& textureName, TextureRegion& region) const;

private:
	unsigned int atlasTextureID;
	int atlasWidth;
	int atlasHeight;
	int textureWidth;
	int textureHeight;
	std::unordered_map<std::string, TextureRegion> textureRegions;

	/**
	 * @brief Deletes any existing atlas texture data.
	 */
	void clearAtlas();
};

#endif // TEXTURE_MANAGER_HPP
