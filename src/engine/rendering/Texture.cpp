#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string &filePath) : textureID(0)
{
	// Détection du format du fichier
	FILE *fp = fopen(filePath.c_str(), "rb");
	if (fp == nullptr)
	{
		std::cerr << "Error: Could not open " << filePath << std::endl;
		return;
	}

	char filecode[8];
	fread(filecode, 1, 8, fp);
	fclose(fp);

	if (strncmp(filecode, "\211PNG\r\n\032\n", 8) == 0) // PNG file header
	{
		if (!loadPNG(filePath))
		{
			std::cerr << "Failed to load texture: " << filePath << std::endl;
		}
	}
	else if (strncmp(filecode, "DDS ", 4) == 0) // DDS file header
	{
		if (!loadDDS(filePath))
		{
			std::cerr << "Failed to load texture: " << filePath << std::endl;
		}
	}
	else
	{
		std::cerr << "Unsupported texture format: " << filePath << std::endl;
	}
}

Texture::~Texture()
{
	if (textureID != 0)
	{
		glDeleteTextures(1, &textureID);
	}
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::loadDDS(const std::string &filePath)
{
	unsigned char header[124];
	FILE *fp = fopen(filePath.c_str(), "rb");
	if (fp == nullptr)
	{
		std::cerr << "Error: Could not open " << filePath << std::endl;
		return false;
	}

	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0)
	{
		fclose(fp);
		std::cerr << "Error: Not a valid DDS file " << filePath << std::endl;
		return false;
	}

	fread(&header, 124, 1, fp);

	unsigned int height = *(unsigned int *)&(header[8]);
	texHeight = height;
	unsigned int width = *(unsigned int *)&(header[12]);
	texWidth = width;
	if (texWidth % 16 != 0 || texHeight % 16 != 0)
	{
		std::cerr << "Error: Texture dimensions should be multiples of block size (16)." << std::endl;
		return false;
	}
	unsigned int linearSize = *(unsigned int *)&(header[16]);
	unsigned int mipMapCount = *(unsigned int *)&(header[24]);
	unsigned int fourCC = *(unsigned int *)&(header[80]);

	unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	unsigned char *buffer = (unsigned char *)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, fp);
	fclose(fp);

	unsigned int format;
	switch (fourCC)
	{
	case 0x31545844: // "DXT1" in ASCII
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case 0x33545844: // "DXT3" in ASCII
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case 0x35545844: // "DXT5" in ASCII
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		std::cerr << "Error: Unsupported DDS format " << filePath << std::endl;
		return false;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);
		offset += size;
		width /= 2;
		height /= 2;

		if (width < 1)
			width = 1;
		if (height < 1)
			height = 1;
	}

	free(buffer);
	return true;
}

bool Texture::loadPNG(const std::string &filePath)
{
	int width, height, channels;
	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
	if (!data)
	{
		std::cerr << "Failed to load PNG file: " << filePath << std::endl;
		return false;
	}

	texWidth = width;
	texHeight = height;

	GLenum format;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA;
	else
	{
		std::cerr << "Unsupported number of channels: " << channels << std::endl;
		stbi_image_free(data);
		return false;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	stbi_image_free(data);

	return true;
}
