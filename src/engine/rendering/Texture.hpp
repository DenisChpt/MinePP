#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <cstring>

class Texture
{
public:
	Texture(const std::string &filePath);
	~Texture();

	void bind() const;
	void unbind() const;
	unsigned int getWidth() const { return texWidth; }
	unsigned int getHeight() const { return texHeight; }

private:
	GLuint textureID;
	unsigned int texWidth, texHeight;

	bool loadDDS(const std::string &filePath);
	bool loadPNG(const std::string &filePath);
};

#endif // TEXTURE_HPP
