#include "Chunk.hpp"
#include <cmath>
#include <mutex>

namespace world {

Chunk::Chunk(int chunkX, int chunkZ)
	: chunkX(chunkX), chunkZ(chunkZ), dirty(true), unload(false)
{
}

Chunk::~Chunk() {
}

int Chunk::getX() const {
	return chunkX;
}

int Chunk::getZ() const {
	return chunkZ;
}

int Chunk::getBlock(int x, int y, int z) const {
	if (x < 0 || x >= CHUNK_SIZE_X || z < 0 || z >= CHUNK_SIZE_Z || y < 0 || y >= CHUNK_HEIGHT) {
		return 0;
	}
	int sectionIndex = y / CHUNK_SECTION_HEIGHT;
	int localY = y % CHUNK_SECTION_HEIGHT;
	return sections[sectionIndex].getBlock(x, localY, z);
}

void Chunk::setBlock(int x, int y, int z, int blockId) {
	if (x < 0 || x >= CHUNK_SIZE_X || z < 0 || z >= CHUNK_SIZE_Z || y < 0 || y >= CHUNK_HEIGHT) {
		return;
	}
	int sectionIndex = y / CHUNK_SECTION_HEIGHT;
	int localY = y % CHUNK_SECTION_HEIGHT;
	sections[sectionIndex].setBlock(x, localY, z, blockId);
	dirty = true;
}

void Chunk::markDirty() {
	dirty = true;
	for (auto& section : sections) {
		section.dirty = true;
	}
}

bool Chunk::shouldUnload() const {
	return unload;
}

void Chunk::setUnload(bool unloadFlag) {
	unload = unloadFlag;
}

static void addQuad(std::vector<Vertex>& vertices, const Vertex& v0, const Vertex& v1,
					const Vertex& v2, const Vertex& v3)
{
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);

	vertices.push_back(v0);
	vertices.push_back(v2);
	vertices.push_back(v3);
}

void Chunk::meshSection(int sectionIndex, std::vector<Vertex>& vertices) {
	int baseY = sectionIndex * CHUNK_SECTION_HEIGHT;

	for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
		for (int ly = 0; ly < CHUNK_SECTION_HEIGHT; ++ly) {
			for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
				int blockId = sections[sectionIndex].getBlock(lx, ly, lz);
				if (blockId == 0) {
					continue;
				}

				int globalX = chunkX * CHUNK_SIZE_X + lx;
				int globalY = baseY + ly;
				int globalZ = chunkZ * CHUNK_SIZE_Z + lz;

				auto isFaceVisible = [this, globalX, globalY, globalZ](int dx, int dy, int dz) -> bool {
					int nx = globalX + dx;
					int ny = globalY + dy;
					int nz = globalZ + dz;
					if (nx < chunkX * CHUNK_SIZE_X || nx >= (chunkX + 1) * CHUNK_SIZE_X ||
						ny < 0 || ny >= CHUNK_HEIGHT ||
						nz < chunkZ * CHUNK_SIZE_Z || nz >= (chunkZ + 1) * CHUNK_SIZE_Z) {
						return true;
					}
					int localX = nx - chunkX * CHUNK_SIZE_X;
					int localZ = nz - chunkZ * CHUNK_SIZE_Z;
					int neighborBlock = getBlock(localX, ny, localZ);
					return (neighborBlock == 0);
				};

				if (isFaceVisible(0, 1, 0)) {
					Vertex v0 { float(globalX),     float(globalY + 1), float(globalZ),     0.0f, 0.0f };
					Vertex v1 { float(globalX + 1), float(globalY + 1), float(globalZ),     1.0f, 0.0f };
					Vertex v2 { float(globalX + 1), float(globalY + 1), float(globalZ + 1), 1.0f, 1.0f };
					Vertex v3 { float(globalX),     float(globalY + 1), float(globalZ + 1), 0.0f, 1.0f };
					addQuad(vertices, v0, v1, v2, v3);
				}

				if (isFaceVisible(0, -1, 0)) {
					Vertex v0 { float(globalX),     float(globalY), float(globalZ),     0.0f, 0.0f };
					Vertex v1 { float(globalX),     float(globalY), float(globalZ + 1), 0.0f, 1.0f };
					Vertex v2 { float(globalX + 1), float(globalY), float(globalZ + 1), 1.0f, 1.0f };
					Vertex v3 { float(globalX + 1), float(globalY), float(globalZ),     1.0f, 0.0f };
					addQuad(vertices, v0, v1, v2, v3);
				}

				if (isFaceVisible(0, 0, -1)) {
					Vertex v0 { float(globalX),     float(globalY),     float(globalZ),     0.0f, 0.0f };
					Vertex v1 { float(globalX + 1), float(globalY),     float(globalZ),     1.0f, 0.0f };
					Vertex v2 { float(globalX + 1), float(globalY + 1), float(globalZ),     1.0f, 1.0f };
					Vertex v3 { float(globalX),     float(globalY + 1), float(globalZ),     0.0f, 1.0f };
					addQuad(vertices, v0, v1, v2, v3);
				}

				if (isFaceVisible(0, 0, 1)) {
					Vertex v0 { float(globalX),     float(globalY),     float(globalZ + 1), 0.0f, 0.0f };
					Vertex v1 { float(globalX),     float(globalY + 1), float(globalZ + 1), 0.0f, 1.0f };
					Vertex v2 { float(globalX + 1), float(globalY + 1), float(globalZ + 1), 1.0f, 1.0f };
					Vertex v3 { float(globalX + 1), float(globalY),     float(globalZ + 1), 1.0f, 0.0f };
					addQuad(vertices, v0, v1, v2, v3);
				}

				if (isFaceVisible(-1, 0, 0)) {
					Vertex v0 { float(globalX),     float(globalY),     float(globalZ),     0.0f, 0.0f };
					Vertex v1 { float(globalX),     float(globalY + 1), float(globalZ),     0.0f, 1.0f };
					Vertex v2 { float(globalX),     float(globalY + 1), float(globalZ + 1), 1.0f, 1.0f };
					Vertex v3 { float(globalX),     float(globalY),     float(globalZ + 1), 1.0f, 0.0f };
					addQuad(vertices, v0, v1, v2, v3);
				}

				if (isFaceVisible(1, 0, 0)) {
					Vertex v0 { float(globalX + 1), float(globalY),     float(globalZ),     0.0f, 0.0f };
					Vertex v1 { float(globalX + 1), float(globalY),     float(globalZ + 1), 1.0f, 0.0f };
					Vertex v2 { float(globalX + 1), float(globalY + 1), float(globalZ + 1), 1.0f, 1.0f };
					Vertex v3 { float(globalX + 1), float(globalY + 1), float(globalZ),     0.0f, 1.0f };
					addQuad(vertices, v0, v1, v2, v3);
				}
			}
		}
	}
}

void Chunk::generateMesh() {
	std::lock_guard<std::mutex> lock(chunkMutex);
	if (!dirty) {
		return;
	}
	mesh.clear();

	for (int i = 0; i < NUM_CHUNK_SECTIONS; ++i) {
		if (sections[i].dirty) {
			meshSection(i, mesh);
			sections[i].dirty = false;
		}
	}
	dirty = false;
}

const std::vector<Vertex>& Chunk::getMesh() const {
	return mesh;
}

} // namespace world
