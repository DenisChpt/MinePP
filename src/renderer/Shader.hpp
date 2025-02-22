#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

/**
 * @brief Encapsulates OpenGL shader program compilation and usage.
 */
class Shader {
public:
	Shader();
	~Shader();

	/**
	 * @brief Loads, compiles, and links vertex and fragment shaders from files.
	 * @param vertexPath Path to the vertex shader file.
	 * @param fragmentPath Path to the fragment shader file.
	 * @return True if successful, otherwise false.
	 */
	bool loadFromFiles(const std::string &vertexPath, const std::string &fragmentPath);

	/**
	 * @brief Activates the shader program for subsequent rendering calls.
	 */
	void use() const;

	/**
	 * @brief Retrieves the underlying OpenGL program identifier.
	 */
	GLuint getProgram() const;

	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
	GLuint program;

	bool compileShader(const std::string &source, GLenum shaderType, GLuint &shaderID);
	std::string readFile(const std::string &filePath);
};

#endif // SHADER_HPP
