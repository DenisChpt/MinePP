#include "Renderer.hpp"
#include <iostream>
#include <glad/glad.h>

namespace renderer {

Renderer::Renderer()
	: opaqueVAO(0), opaqueVBO(0),
	  transparentVAO(0), transparentVBO(0),
	  foliageVAO(0), foliageVBO(0),
	  opaqueVertexCount(0), transparentVertexCount(0), foliageVertexCount(0)
{
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &opaqueVAO);
	glDeleteBuffers(1, &opaqueVBO);
	glDeleteVertexArrays(1, &transparentVAO);
	glDeleteBuffers(1, &transparentVBO);
	glDeleteVertexArrays(1, &foliageVAO);
	glDeleteBuffers(1, &foliageVBO);
}

bool Renderer::initialize() {
	opaqueShader = std::make_unique<Shader>();
	if (!opaqueShader->loadFromFiles("shaders/opaque.vert", "shaders/opaque.frag")) {
		std::cerr << "Error loading opaque shaders." << std::endl;
		return false;
	}

	transparentShader = std::make_unique<Shader>();
	if (!transparentShader->loadFromFiles("shaders/transparent.vert", "shaders/transparent.frag")) {
		std::cerr << "Error loading transparent shaders." << std::endl;
		return false;
	}

	foliageShader = std::make_unique<Shader>();
	if (!foliageShader->loadFromFiles("shaders/foliage.vert", "shaders/foliage.frag")) {
		std::cerr << "Error loading foliage shaders." << std::endl;
		return false;
	}

	glGenVertexArrays(1, &opaqueVAO);
	glGenBuffers(1, &opaqueVBO);

	glGenVertexArrays(1, &transparentVAO);
	glGenBuffers(1, &transparentVBO);

	glGenVertexArrays(1, &foliageVAO);
	glGenBuffers(1, &foliageVBO);

	return true;
}

void Renderer::setupMesh(GLuint &VAO, GLuint &VBO, const std::vector<float>& vertices) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Renderer::setOpaqueMesh(const std::vector<float>& vertices) {
	opaqueVertexCount = vertices.size() / 5;
	setupMesh(opaqueVAO, opaqueVBO, vertices);
}

void Renderer::setTransparentMesh(const std::vector<float>& vertices) {
	transparentVertexCount = vertices.size() / 5;
	setupMesh(transparentVAO, transparentVBO, vertices);
}

void Renderer::setFoliageMesh(const std::vector<float>& vertices) {
	foliageVertexCount = vertices.size() / 5;
	setupMesh(foliageVAO, foliageVBO, vertices);
}

void Renderer::render(const glm::mat4 &view, const glm::mat4 &projection) {
	// Opaque
	opaqueShader->use();
	GLuint program = opaqueShader->getProgram();
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projection[0][0]);

	glBindVertexArray(opaqueVAO);
	glDrawArrays(GL_TRIANGLES, 0, opaqueVertexCount);
	glBindVertexArray(0);

	// Transparent
	transparentShader->use();
	program = transparentShader->getProgram();
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projection[0][0]);

	glBindVertexArray(transparentVAO);
	glDrawArrays(GL_TRIANGLES, 0, transparentVertexCount);
	glBindVertexArray(0);

	// Foliage
	foliageShader->use();
	program = foliageShader->getProgram();
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projection[0][0]);

	glBindVertexArray(foliageVAO);
	glDrawArrays(GL_TRIANGLES, 0, foliageVertexCount);
	glBindVertexArray(0);
}

} // namespace renderer
