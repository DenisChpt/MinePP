/**
 * @class Texture
 * @brief Encapsule la création et la gestion des textures OpenGL.
 *
 * @details La classe Texture permet de créer et d'allouer des textures 2D, des tableaux de textures 2D et des cubemaps.
 *          Elle offre des méthodes pour uploader des données d'image, générer des mipmaps et lier la texture à une unité.
 *
 * @param target Cible OpenGL (GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY ou GL_TEXTURE_CUBE_MAP).
 * @param internalFormat Format interne de la texture.
 * @param format Format des données.
 * @param type Type de donnée.
 * @param generateMipMap Indique si des mipmaps doivent être générés.
 */


#pragma once

#include "../MinePP.hpp"
#include "Image.hpp"

class Texture
{
	uint32_t id = 0;
	uint32_t target;
	int32_t internalFormat;
	int32_t format;
	int32_t type;
	bool generateMipMap;

public:
	Texture(uint32_t target,
			int32_t internalFormat,
			int32_t format,
			int32_t type,
			bool generateMipMap = true,
			int32_t maxLod = 1000);
	~Texture();

	[[nodiscard]] bool isValid() const { return id != 0; };

	void bind() const;
	void bindToSlot(uint32_t slot) const;
	void unbind() const;

	[[nodiscard]] uint32_t getId() const { return id; }

	void allocateTexture(int32_t width, int32_t height);
	void buffer2DRGBAData(const Image &image);
	void buffer2DArrayRGBAData(std::span<const Image> images);
	void bufferCubeMapRGBAData(std::span<Ref<const Image>, 6> images);

	static Ref<const Texture> loadTexture2D(const std::string &name);
	static Ref<const Texture> loadTexture2DArray(const std::string &name);
	static Ref<const Texture> loadCubeMapTexture(const std::string &name);

	static Ref<const Texture> loadTexture2DArrayFromImages(const std::vector<Image>& images, int tileWidth, int tileHeight);

	Texture(const Texture &) = delete;
	Texture(Texture &) = delete;
	Texture(Texture &&) noexcept = delete;
	Texture &operator=(Texture &) = delete;
	Texture &operator=(Texture &&) noexcept = delete;
};
