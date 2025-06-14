#pragma once

#include "../Common.hpp"

// Forward declaration
class Assets;

// Image data structure
struct Image {
	uint32_t width, height;
	std::vector<uint8_t> data;

	Image subImage(glm::uvec2 offset, glm::uvec2 extent) const;
};

// Texture class
class Texture {
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
	void buffer2DRGBAData(const Image& image);
	void buffer2DArrayRGBAData(std::span<const Image> images);
	void bufferCubeMapRGBAData(std::span<Ref<const Image>, 6> images);

	static Ref<const Texture> loadTexture2D(const std::string& name, Assets& assets);
	static Ref<const Texture> loadTexture2DArray(const std::string& name, Assets& assets);
	static Ref<const Texture> loadCubeMapTexture(const std::string& name, Assets& assets);

	static Ref<const Texture> loadTexture2DArrayFromImages(const std::vector<Image>& images,
														   int tileWidth,
														   int tileHeight);

	Texture(const Texture&) = delete;
	Texture(Texture&) = delete;
	Texture(Texture&&) noexcept = delete;
	Texture& operator=(Texture&) = delete;
	Texture& operator=(Texture&&) noexcept = delete;
};