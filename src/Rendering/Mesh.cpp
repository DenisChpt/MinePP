#include "Mesh.hpp"

// BlockMesh static data
const std::array<std::array<BlockVertex, 6>, 6> BlockMesh::vertices = {{
	{{
		// top
		{{0, 1, 1}, {1, 0}},
		{{1, 1, 1}, {1, 1}},
		{{0, 1, 0}, {0, 0}},
		{{1, 1, 1}, {1, 1}},
		{{1, 1, 0}, {0, 1}},
		{{0, 1, 0}, {0, 0}},
	}},
	{{
		// +x east
		{{1, 1, 1}, {1, 0}},
		{{1, 0, 1}, {1, 1}},
		{{1, 1, 0}, {0, 0}},
		{{1, 0, 1}, {1, 1}},
		{{1, 0, 0}, {0, 1}},
		{{1, 1, 0}, {0, 0}},
	}},
	{{
		//-x west
		{{0, 1, 0}, {1, 0}},
		{{0, 0, 0}, {1, 1}},
		{{0, 1, 1}, {0, 0}},
		{{0, 0, 0}, {1, 1}},
		{{0, 0, 1}, {0, 1}},
		{{0, 1, 1}, {0, 0}},
	}},
	{{
		//-z north
		{{1, 1, 0}, {1, 0}},
		{{1, 0, 0}, {1, 1}},
		{{0, 1, 0}, {0, 0}},
		{{1, 0, 0}, {1, 1}},
		{{0, 0, 0}, {0, 1}},
		{{0, 1, 0}, {0, 0}},
	}},

	{{
		// +z south
		{{0, 1, 1}, {1, 0}},
		{{0, 0, 1}, {1, 1}},
		{{1, 1, 1}, {0, 0}},
		{{0, 0, 1}, {1, 1}},
		{{1, 0, 1}, {0, 1}},
		{{1, 1, 1}, {0, 0}},
	}},
	{{
		// bottom
		{{1, 0, 1}, {1, 0}},
		{{0, 0, 1}, {1, 1}},
		{{1, 0, 0}, {0, 0}},
		{{0, 0, 1}, {1, 1}},
		{{0, 0, 0}, {0, 1}},
		{{1, 0, 0}, {0, 0}},
	}},
}};

// CubeMesh implementation
CubeMesh::CubeMesh()
{
	std::vector<BlockVertex> vertices;

	vertices.resize(6 * 6);
	int32_t vertexCount = 0;
	for (const auto &face : BlockMesh::vertices)
	{
		for (const auto &vertex : face)
		{
			vertices.at(vertexCount) = vertex;
			vertexCount++;
		}
	}
	cubeVertexArray = std::make_shared<VertexArray>(vertices, BlockVertex::vertexAttributes());
}

void CubeMesh::render() const
{
	cubeVertexArray->renderVertexStream();
}