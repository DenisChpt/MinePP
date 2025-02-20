#ifndef POST_PROCESSOR_HPP
#define POST_PROCESSOR_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.hpp"

namespace renderer {

class PostProcessor {
public:
	PostProcessor();
	~PostProcessor();

	// Initialise le post-processor : framebuffer, texture, quad écran
	bool initialize(int screenWidth, int screenHeight);

	// Lie le framebuffer pour le rendu de la scène
	void bindFramebuffer() const;

	// Exécute le passage de post‑traitement et affiche l’image finale
	void render(float time);

	// Redimensionne le framebuffer
	void resize(int screenWidth, int screenHeight);

private:
	GLuint FBO, textureColorBuffer, RBO;
	GLuint quadVAO, quadVBO;
	std::unique_ptr<Shader> postShader;
	int screenWidth, screenHeight;

	void initRenderData();
};

} // namespace renderer

#endif // POST_PROCESSOR_HPP
