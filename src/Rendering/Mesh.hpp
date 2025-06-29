/**
 * @file Mesh.hpp
 * @brief Gestion unifiée des maillages de blocs et de cubes
 *
 * @details Fusionne BlockMesh et CubeMesh pour simplifier la gestion des maillages
 */

#pragma once

#include "BlockVertex.hpp"
#include "Buffers.hpp"

/**
 * @class BlockMesh
 * @brief Génère le maillage d'un bloc à partir d'un ensemble statique de sommets
 */
struct BlockMesh {
	static const std::array<BlockVertex, 6>& getVerticesFromDirection(const glm::ivec3& vec) {
		assert(glm::abs(vec.x) + glm::abs(vec.y) + glm::abs(vec.z) == 1);

		switch (vec.x) {
			case 1:
				return vertices[1];
			case -1:
				return vertices[2];
			default:
				break;
		}

		switch (vec.y) {
			case 1:
				return vertices[0];
			case -1:
				return vertices[5];
			default:
				break;
		}

		switch (vec.z) {
			case 1:
				return vertices[4];
			case -1:
				return vertices[3];
			default:
				break;
		}

		assert(false);
		return vertices[0];
	}

	static const std::array<std::array<BlockVertex, 6>, 6> vertices;
};

/**
 * @class CubeMesh
 * @brief Génère un maillage de cube pour l'affichage d'effets (ex. contours de bloc)
 */
class CubeMesh {
	Ref<VertexArray> cubeVertexArray;
	Ref<VertexBuffer> vertexBuffer;
	GLsizei vertexCount = 0;

   public:
	CubeMesh();
	void render() const;
	
	// For instanced rendering
	const VertexArray& getVertexArray() const { return *cubeVertexArray; }
	const VertexBuffer& getVertexBuffer() const { return *vertexBuffer; }
	GLsizei getVertexCount() const { return vertexCount; }
};