#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

namespace renderer {

class Renderer {
public:
	Renderer();
	~Renderer();

	// Initialise le renderer : chargement des shaders et configuration des VAO/VBO
	bool initialize();

	// Fonction de rendu : reçoit les matrices de vue et projection
	void render(const glm::mat4 &view, const glm::mat4 &projection);

	// Fournit les vertex buffers pour chaque pipeline de rendu
	void setOpaqueMesh(const std::vector<float>& vertices);
	void setTransparentMesh(const std::vector<float>& vertices);
	void setFoliageMesh(const std::vector<float>& vertices);

private:
	// Shader pour chaque pipeline
	std::unique_ptr<Shader> opaqueShader;
	std::unique_ptr<Shader> transparentShader;
	std::unique_ptr<Shader> foliageShader;

	// Objets OpenGL pour les VAO/VBO des différents types de mesh
	GLuint opaqueVAO, opaqueVBO;
	GLuint transparentVAO, transparentVBO;
	GLuint foliageVAO, foliageVBO;

	// Nombre de vertex par pipeline (on suppose 5 floats par vertex : 3 pour la position et 2 pour les UV)
	size_t opaqueVertexCount;
	size_t transparentVertexCount;
	size_t foliageVertexCount;

	// Méthode d'aide pour configurer un VAO/VBO à partir d'un tableau de vertex
	void setupMesh(GLuint &VAO, GLuint &VBO, const std::vector<float>& vertices);
};

} // namespace renderer

#endif // RENDERER_HPP
