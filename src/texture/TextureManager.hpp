#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace polymer {
namespace texture {

	/**
	 * @brief Defines a UV region for a texture within an atlas.
	 */
	struct TextureRegion {
		glm::vec2 uvMin;
		glm::vec2 uvMax;
	};

	/**
	 * @brief Manages multiple individual images, packing them into a single atlas texture.
	 */
	class TextureManager {
	public:
		TextureManager();
		~TextureManager();

		/**
		 * @brief Loads an image from a file for later inclusion in the atlas.
		 * @param filePath Path to the image file.
		 * @return The TextureRegion assigned (initially uninitialized).
		 */
		TextureRegion loadTexture(const std::string& filePath);

		/**
		 * @brief Builds the final atlas texture after all images are loaded.
		 * @return True on success, otherwise false.
		 */
		bool buildAtlas();

		/**
		 * @brief Gets the OpenGL texture ID of the built atlas.
		 */
		GLuint getAtlasTextureID() const { return atlasTextureID; }

	private:
		struct ImageData {
			int width;
			int height;
			int channels;
			unsigned char* data;
			std::string filePath;
		};

		std::vector<ImageData> images;
		std::unordered_map<std::string, TextureRegion> textureRegions;

		GLuint atlasTextureID;
		int atlasWidth;
		int atlasHeight;

		void freeImages();
		bool packImages();
	};

} // namespace texture
} // namespace polymer

#endif // TEXTURE_MANAGER_HPP
