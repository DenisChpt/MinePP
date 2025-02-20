#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
	Shader();
	~Shader();

	// Charge et compile les shaders à partir des fichiers spécifiés
	bool loadFromFiles(const std::string &vertexPath, const std::string &fragmentPath);

	// Utilise le programme shader courant
	void use() const;

	// Retourne l'identifiant du programme shader
	GLuint getProgram() const;

	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
	GLuint program;

	// Compile un shader depuis une source en mémoire
	bool compileShader(const std::string &source, GLenum shaderType, GLuint &shaderID);

	// Lit le contenu d'un fichier et renvoie une chaîne
	std::string readFile(const std::string &filePath);
};

#endif // SHADER_HPP
