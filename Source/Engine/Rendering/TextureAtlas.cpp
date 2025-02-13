#include "TextureAtlas.hpp"
#include <iostream>
#include <stb_image.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

TextureAtlas::TextureAtlas(const std::string& filePath, unsigned int gridSize)
	: m_FilePath(filePath), m_GridSize(gridSize), m_TextureID(0),
	  m_AtlasWidth(0), m_AtlasHeight(0), m_Loaded(false) {}

TextureAtlas::~TextureAtlas() {
	if (m_TextureID != 0) {
		glDeleteTextures(1, &m_TextureID);
	}
}

bool TextureAtlas::load() {
	int channels;
	unsigned char* data = stbi_load(m_FilePath.c_str(), &m_AtlasWidth, &m_AtlasHeight, &channels, 0);
	if (!data) {
		std::cerr << "[TextureAtlas] Erreur lors du chargement de " << m_FilePath << std::endl;
		return false;
	}

	GLenum format = GL_RGB;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA;

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, m_AtlasWidth, m_AtlasHeight, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Réglages de filtrage et d'emballage
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	stbi_image_free(data);
	m_Loaded = true;
	std::cout << "[TextureAtlas] Atlas chargé (" << m_FilePath << ")." << std::endl;
	return true;
}

unsigned int TextureAtlas::getTextureID() const {
	return m_TextureID;
}

std::vector<glm::vec2> TextureAtlas::getUVCoordinates(unsigned int index) const {
	std::vector<glm::vec2> uvs(4);
	if (!m_Loaded || m_GridSize == 0) return uvs;

	// Calculer la taille d'une cellule en UV (entre 0 et 1)
	float cellSizeU = 1.0f / static_cast<float>(m_GridSize);
	float cellSizeV = 1.0f / static_cast<float>(m_GridSize);

	// Déduire la position dans la grille
	unsigned int x = index % m_GridSize;
	unsigned int y = index / m_GridSize;

	// Attention : en OpenGL l'origine des UV est en bas à gauche.
	float u_min = x * cellSizeU;
	float v_min = y * cellSizeV;
	float u_max = u_min + cellSizeU;
	float v_max = v_min + cellSizeV;

	// On retourne les 4 coins : bottom-left, bottom-right, top-right, top-left
	uvs[0] = glm::vec2(u_min, v_min);
	uvs[1] = glm::vec2(u_max, v_min);
	uvs[2] = glm::vec2(u_max, v_max);
	uvs[3] = glm::vec2(u_min, v_max);
	return uvs;
}
