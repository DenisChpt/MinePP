#include "Chunk.hpp"
#include <iostream>
#include <cmath>   // floor
#include <cstring> // memset

#include "BlockType.hpp" // Pour la déf. du BlockType, isOpaque(), etc.

namespace Game {
namespace World {

Chunk::Chunk(int chunkX, int chunkZ)
	: m_chunkX(chunkX)
	, m_chunkZ(chunkZ)
	, m_vao(0)
	, m_vbo(0)
	, m_vertexCount(0)
{
	// Initialiser tous les blocs à Air
	memset(m_blocks, (int)BlockType::Air, sizeof(m_blocks));
}

Chunk::~Chunk()
{
	if (m_vbo) glDeleteBuffers(1, &m_vbo);
	if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Chunk::setBlock(int x, int y, int z, BlockType type)
{
	if(x<0 || x>=CHUNK_SIZE || y<0 || y>=CHUNK_HEIGHT || z<0|| z>=CHUNK_SIZE)
		return;
	m_blocks[x][y][z] = type;
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
	if(x<0 || x>=CHUNK_SIZE || y<0 || y>=CHUNK_HEIGHT || z<0|| z>=CHUNK_SIZE)
		return BlockType::Air;
	return m_blocks[x][y][z];
}

BlockType Chunk::getBlockSafe(int x, int y, int z) const
{
	// Dans un vrai jeu, on irait chercher le chunk voisin si x ou z sort de [0..CHUNK_SIZE-1].
	if (x < 0 || x >= CHUNK_SIZE ||
		y < 0 || y >= CHUNK_HEIGHT ||
		z < 0 || z >= CHUNK_SIZE)
	{
		// Hors de ce chunk : renvoyer Air pour éviter de masquer la face
		return BlockType::Air;
	}
	return m_blocks[x][y][z];
}

// ==================================================================
// getUV : similaire à ce que tu faisais avec l'atlas dans Block::render
// ==================================================================
glm::vec4 Chunk::getUV(const Atlas& atlas, const std::string& textureID) const
{
	auto it = atlas.mapping.find(textureID);
	if(it == atlas.mapping.end()) {
		// Texture non trouvée : on retourne tout l'atlas (0..1), c'est mieux que rien
		return glm::vec4(0,0,1,1);
	}
	const AtlasEntry& entry = it->second;
	float u_min = entry.x / float(atlas.atlasWidth);
	float v_min = entry.y / float(atlas.atlasHeight);
	float u_max = (entry.x + entry.width) / float(atlas.atlasWidth);
	float v_max = (entry.y + entry.height) / float(atlas.atlasHeight);
	return glm::vec4(u_min, v_min, u_max, v_max);
}

/**
 * @brief getTextureForBlock : pour associer un BlockType et une face (front/back/left/right/top/bottom)
 *        à l'ID de texture souhaitée.
 *
 *        Ici, on reproduit la logique "GrassBlock" => grass_top, grass_side, dirt, etc.
 *        À adapter selon tes assets.
 *
 * @param type     : Le type de bloc (Grass, Dirt, Stone, etc.)
 * @param faceIdx  : 0=front, 1=back, 2=left, 3=right, 4=top, 5=bottom (comme dans l'ancien code).
 */
std::string Chunk::getTextureForBlock(BlockType type, int faceIdx) const
{
	switch(type) {
		case BlockType::Air:
			return ""; // invisible
		case BlockType::Stone:
			return "stone"; // même texture pour toutes les faces
		case BlockType::Dirt:
			return "dirt";  // idem
		case BlockType::Grass:
			// faceIdx : 4=top, 5=bottom, else side
			if(faceIdx == 4) return "grass_top";
			if(faceIdx == 5) return "dirt";
			return "grass_side";
		case BlockType::Water:
			return "water";
		case BlockType::Leaves:
			return "leaves";
		case BlockType::Log:
			// Pour un bloc "Wood" type "log", top et bottom -> "log_top", le reste -> "log"
			if(faceIdx == 4 || faceIdx == 5) return "log_top";
			return "log";
		default:
			return "error";
	}
}

// ==================================================================
//  generateMesh : reproduit l'ordre EXACT de tes anciens cubes
// ==================================================================
void Chunk::generateMesh(const Atlas& atlas)
{
	if (!m_vao) {
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
	}
	std::vector<Vertex> vertices;
	vertices.reserve(50000); // estimation large

	// Parcours de tous les blocs
	for(int x=0; x<CHUNK_SIZE; x++){
		for(int y=0; y<CHUNK_HEIGHT; y++){
			for(int z=0; z<CHUNK_SIZE; z++){
				BlockType type = getBlock(x,y,z);
				if(type == BlockType::Air) continue;

				// Pour chaque face (0=front, 1=back, 2=left, 3=right, 4=top, 5=bottom)
				for(int face=0; face<6; face++){
					// Calcul du bloc voisin dans la direction de la face
					// (pour voir s'il est opaque => masquer)
					// Ordre hérité de ton "Block::render"
					// 0=front (z+), 1=back (z-), 2=left (x-), 3=right (x+), 4=top (y+), 5=bottom (y-)

					int nx = x, ny = y, nz = z;
					switch(face) {
						case 0: nz = z+1; break; // front
						case 1: nz = z-1; break; // back
						case 2: nx = x+1; break; // left
						case 3: nx = x-1; break; // right
						case 4: ny = y+1; break; // top
						case 5: ny = y-1; break; // bottom
					}

					BlockType neighborType = getBlockSafe(nx, ny, nz);
					if(isOpaque(neighborType) && !isTransparent(neighborType)) {
						// Face masquée -> skip
						continue;
					}

					// Récupérer la texture pour cette face
					std::string texID = getTextureForBlock(type, face);
					if(texID.empty()) continue; // bloc invisible

					// Récupérer UV
					glm::vec4 uv = getUV(atlas, texID);

					// Coordonnées réelles dans le monde
					float wx = float(m_chunkX*CHUNK_SIZE + x);
					float wy = float(y);
					float wz = float(m_chunkZ*CHUNK_SIZE + z);

					// Calcul des 8 sommets du cube, centré sur (wx,wy,wz) + halfSize=0.5
					glm::vec3 p0(wx,   wy,   wz+1);
					glm::vec3 p1(wx+1, wy,   wz+1);
					glm::vec3 p2(wx+1, wy+1, wz+1);
					glm::vec3 p3(wx,   wy+1, wz+1);
					glm::vec3 p4(wx,   wy,   wz);
					glm::vec3 p5(wx+1, wy,   wz);
					glm::vec3 p6(wx+1, wy+1, wz);
					glm::vec3 p7(wx,   wy+1, wz);

					// Ajout des 2 triangles de la face correspondante
					auto pushFace = [&](const glm::vec3& A,
										const glm::vec3& B,
										const glm::vec3& C,
										const glm::vec3& D,
										const glm::vec4& uvRect) {
						// Triangle 1 : A, B, C
						vertices.push_back({A.x, A.y, A.z, uvRect.x, uvRect.y});
						vertices.push_back({B.x, B.y, B.z, uvRect.z, uvRect.y});
						vertices.push_back({C.x, C.y, C.z, uvRect.z, uvRect.w});

						// Triangle 2 : C, D, A
						vertices.push_back({C.x, C.y, C.z, uvRect.z, uvRect.w});
						vertices.push_back({D.x, D.y, D.z, uvRect.x, uvRect.w});
						vertices.push_back({A.x, A.y, A.z, uvRect.x, uvRect.y});
					};

					// Reprend l'ordre EXACT de ton ancien code :
					switch(face) {
						case 0: // front  (z+)
							pushFace(p3, p2, p1, p0, uv);
							break;
						case 1: // back   (z-)
							pushFace(p6, p7, p4, p5, uv);
							break;
						case 2: // left   (x-)
							pushFace(p2, p6, p5, p1, uv);
							break;
						case 3: // right  (x+)
							pushFace(p7, p3, p0, p4, uv);
							break;
						case 4: // top    (y+)
							pushFace(p7, p6, p2, p3, uv);
							break;
						case 5: // bottom (y-)
							pushFace(p4, p5, p1, p0, uv);
							break;
					}
				}
			}
		}
	}

	// Upload du tableau
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	m_vertexCount = (GLsizei)vertices.size();

	glBindVertexArray(0);

	std::cout << "[Chunk] generateMesh => " << m_vertexCount << " vertices.\n";
}

void Chunk::render() const
{
	if(m_vertexCount == 0) return;
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
	glBindVertexArray(0);
}

// ===========================================================================
// placeTree : génère un arbre type "chêne" dans ce chunk, à la position (x,y,z)
// ===========================================================================
void Chunk::placeTree(int xBase, int yBase, int zBase)
{
	// Petit contrôle
	if (yBase < 0 || yBase >= CHUNK_HEIGHT)
		return;

	// Hauteur aléatoire [4..6]
	int trunkHeight = 4 + (std::rand() % 3);

	// On place le tronc (Wood)
	for(int h=0; h<trunkHeight; h++){
		int y = yBase + h;
		if(y >= CHUNK_HEIGHT) break;
		setBlock(xBase, y, zBase, BlockType::Log);
	}

	// Position du sommet du tronc
	int topY = yBase + trunkHeight;

	// On place un feuillage en "carré" sur 2 couches
	// ex : 2 couches => y= topY, topY+1
	for(int layer=0; layer<2; layer++){
		int ly = topY + layer;
		if(ly >= CHUNK_HEIGHT) break;
		// un 3x3 autour
		for(int dx=-1; dx<=1; dx++){
			for(int dz=-1; dz<=1; dz++){
				// Option: retirer les coins sur la couche du haut
				if(layer==1 && (std::abs(dx)+std::abs(dz) == 2)) {
					// skip corners
					continue;
				}
				int lx = xBase + dx;
				int lz = zBase + dz;
				// On évite d’écraser un bloc si c’est déjà Wood ou autre
				// (optionnel, selon ta logique)
				if(getBlockSafe(lx, ly, lz) == BlockType::Air){
					setBlock(lx, ly, lz, BlockType::Leaves);
				}
			}
		}
	}
}
}
}