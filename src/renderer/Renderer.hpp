#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

namespace renderer {

	/**
	 * @brief Coordinates the rendering of opaque, transparent, and foliage geometry.
	 */
	class Renderer {
	public:
		Renderer();
		~Renderer();

		/**
		 * @brief Initializes the renderer by setting up shaders and GPU buffers.
		 */
		bool initialize();

		/**
		 * @brief Renders all geometry using the specified view and projection matrices.
		 */
		void render(const glm::mat4 &view, const glm::mat4 &projection);

		/**
		 * @brief Sets the vertex data for opaque objects.
		 */
		void setOpaqueMesh(const std::vector<float>& vertices);

		/**
		 * @brief Sets the vertex data for transparent objects.
		 */
		void setTransparentMesh(const std::vector<float>& vertices);

		/**
		 * @brief Sets the vertex data for foliage objects.
		 */
		void setFoliageMesh(const std::vector<float>& vertices);

	private:
		std::unique_ptr<Shader> opaqueShader;
		std::unique_ptr<Shader> transparentShader;
		std::unique_ptr<Shader> foliageShader;

		GLuint opaqueVAO, opaqueVBO;
		GLuint transparentVAO, transparentVBO;
		GLuint foliageVAO, foliageVBO;

		size_t opaqueVertexCount;
		size_t transparentVertexCount;
		size_t foliageVertexCount;

		/**
		 * @brief Helper that uploads mesh data to a VAO/VBO.
		 */
		void setupMesh(GLuint &VAO, GLuint &VBO, const std::vector<float>& vertices);
	};

} // namespace renderer

#endif // RENDERER_HPP
