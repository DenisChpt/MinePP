#include "Shader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader() : program(0) {
}

Shader::~Shader() {
	if (program) {
		glDeleteProgram(program);
	}
}

std::string Shader::readFile(const std::string &filePath) {
	std::ifstream file(filePath);
	if (!file) {
		std::cerr << "Error opening shader file: " << filePath << std::endl;
		return "";
	}
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

bool Shader::compileShader(const std::string &source, GLenum shaderType, GLuint &shaderID) {
	shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();
	glShaderSource(shaderID, 1, &src, nullptr);
	glCompileShader(shaderID);

	GLint success;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
		std::cerr << "Shader compile error (" 
				  << ((shaderType == GL_VERTEX_SHADER) ? "vertex" : "fragment")
				  << "): " << infoLog << std::endl;
		return false;
	}
	return true;
}

bool Shader::loadFromFiles(const std::string &vertexPath, const std::string &fragmentPath) {
	std::string vertexSource = readFile(vertexPath);
	std::string fragmentSource = readFile(fragmentPath);
	if (vertexSource.empty() || fragmentSource.empty()) {
		return false;
	}

	GLuint vertexShader, fragmentShader;
	if (!compileShader(vertexSource, GL_VERTEX_SHADER, vertexShader)) {
		return false;
	}
	if (!compileShader(fragmentSource, GL_FRAGMENT_SHADER, fragmentShader)) {
		glDeleteShader(vertexShader);
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "Shader program link error: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return true;
}

void Shader::use() const {
	glUseProgram(program);
}

GLuint Shader::getProgram() const {
	return program;
}

void Shader::setBool(const std::string &name, bool value) const {
	glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
	glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
	glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
	glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
