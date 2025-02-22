#ifndef POST_PROCESSOR_HPP
#define POST_PROCESSOR_HPP

#include <glm/glm.hpp>
#include <memory>
#include "Shader.hpp"

namespace renderer {

	/**
	 * @brief Applies post-processing effects to a rendered scene.
	 */
	class PostProcessor {
	public:
		PostProcessor();
		~PostProcessor();

		/**
		 * @brief Sets up an off-screen framebuffer for post-processing.
		 */
		bool initialize(int screenWidth, int screenHeight);

		/**
		 * @brief Binds the internal framebuffer for rendering.
		 */
		void bindFramebuffer() const;

		/**
		 * @brief Renders the final post-processed image to the screen.
		 */
		void render(float time);

		/**
		 * @brief Resizes the associated framebuffer and textures.
		 */
		void resize(int screenWidth, int screenHeight);

	private:
		GLuint FBO, textureColorBuffer, RBO;
		GLuint quadVAO, quadVBO;
		std::unique_ptr<Shader> postShader;
		int screenWidth, screenHeight;

		/**
		 * @brief Initializes the geometry for the full-screen quad.
		 */
		void initRenderData();
	};

} // namespace renderer

#endif // POST_PROCESSOR_HPP
