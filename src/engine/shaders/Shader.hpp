#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
public:
	Shader();
	~Shader();

	void compile(const GLchar* vertexSource, const GLchar* fragmentSource);
	void use() const;

	void setMatrix4(const std::string& name, const glm::mat4& matrix) const;
	void setInt(const std::string& name, int value) const;  // Ajoutez cette méthode

private:
	GLuint programID;

	void checkCompileErrors(GLuint shader, std::string type);
};

#endif // SHADER_HPP
