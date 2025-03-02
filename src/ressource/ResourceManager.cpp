#include "ResourceManager.hpp"
#include "Utils.hpp"
#include <iostream>

// Nous utilisons des maps statiques pour mettre en cache les textures et programmes.
namespace ResourceManager {

	static std::map<std::string, GLuint> textureCache;
	static std::map<std::string, GLuint> programCache;

	GLuint loadTexture(const std::string &filePath) {
		// Vérifier si la texture est déjà chargée.
		if (textureCache.find(filePath) != textureCache.end()) {
			return textureCache[filePath];
		}
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Définir les paramètres de filtrage.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Charger l'image PNG depuis le fichier.
		Utils::load_png_texture(filePath.c_str());
		
		// Débind la texture.
		glBindTexture(GL_TEXTURE_2D, 0);

		// Mettre en cache et retourner l'ID.
		textureCache[filePath] = texture;
		std::cout << "[ResourceManager] Texture chargée: " << filePath << std::endl;
		return texture;
	}

	GLuint loadShader(GLenum type, const std::string &filePath) {
		// La fonction Utils::load_shader attend un type et un chemin.
		GLuint shader = Utils::load_shader(type, filePath.c_str());
		if (shader == 0) {
			std::cerr << "[ResourceManager] Erreur lors du chargement du shader: " << filePath << std::endl;
		} else {
			std::cout << "[ResourceManager] Shader chargé: " << filePath << std::endl;
		}
		return shader;
	}

	GLuint loadProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
		// Utiliser un identifiant unique pour le programme (concaténation des deux chemins).
		std::string programKey = vertexShaderPath + ";" + fragmentShaderPath;
		if (programCache.find(programKey) != programCache.end()) {
			return programCache[programKey];
		}
		// Charger les shaders.
		GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderPath);
		GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderPath);
		if (vertexShader == 0 || fragmentShader == 0) {
			std::cerr << "[ResourceManager] Échec du chargement des shaders pour le programme." << std::endl;
			return 0;
		}
		// Créer et lier le programme.
		GLuint program = Utils::load_program(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
		if (program == 0) {
			std::cerr << "[ResourceManager] Erreur lors du linkage du programme shader." << std::endl;
		} else {
			std::cout << "[ResourceManager] Programme shader créé: " << programKey << std::endl;
		}
		// Mettre en cache.
		programCache[programKey] = program;
		return program;
	}

	void clear() {
		// Libérer les textures.
		for (auto &pair : textureCache) {
			glDeleteTextures(1, &pair.second);
		}
		textureCache.clear();

		// Libérer les programmes.
		for (auto &pair : programCache) {
			glDeleteProgram(pair.second);
		}
		programCache.clear();
	}

} // namespace ResourceManager
