#include "WorldGenerator.hpp"
#include <cmath>
#include <random>
#include <iostream>

namespace Game {

// ====================================
//        Constructeur/Destructeur
// ====================================
WorldGenerator::WorldGenerator(unsigned int seed)
	: m_seed(seed)
{
}

WorldGenerator::~WorldGenerator()
{
}

// ====================================
//    Bruit Perlin 2D (simplifié)
// ====================================
static int g_perm[512];
static bool g_permInitialized = false;

// Init permutation (une seule fois par seed)
static void initPermutation(unsigned int seed) {
	if(g_permInitialized) return;
	g_permInitialized = true;

	std::mt19937 rng(seed);
	std::vector<int> p(256);
	for(int i=0; i<256; i++) p[i] = i;
	// shuffle
	for(int i=255; i>0; i--){
		std::uniform_int_distribution<int> dist(0, i);
		int r = dist(rng);
		std::swap(p[i], p[r]);
	}
	// dupliquer
	for(int i=0; i<512; i++){
		g_perm[i] = p[i & 255];
	}
}

// fonctions fade, grad...
static float fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}
static float grad(int hash, float x, float y) {
	int h = hash & 3; 
	float u = (h < 2) ? x : y;
	float v = (h < 2) ? y : x;
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float WorldGenerator::perlin2D(float x, float y, int repeat) const
{
	// Initialiser la permutation (une seule fois)
	initPermutation(m_seed);

	// Si on veut répéter le motif => x mod repeat, etc. (optionnel)
	if (repeat > 0) {
		x = fmod(x, repeat);
		y = fmod(y, repeat);
	}

	int xi = (int)floor(x) & 255;
	int yi = (int)floor(y) & 255;

	float xf = x - floor(x);
	float yf = y - floor(y);

	int aa = g_perm[xi + g_perm[yi]];
	int ab = g_perm[xi + g_perm[yi + 1]];
	int ba = g_perm[xi + 1 + g_perm[yi]];
	int bb = g_perm[xi + 1 + g_perm[yi + 1]];

	float u = fade(xf);
	float v = fade(yf);

	float x1, x2, y1, y2;
	x1 = grad(aa, xf, yf);
	x2 = grad(ba, xf - 1, yf);
	y1 = (1.0f - u) * x1 + u * x2;

	x1 = grad(ab, xf, yf - 1);
	x2 = grad(bb, xf - 1, yf - 1);
	y2 = (1.0f - u) * x1 + u * x2;

	return ((1.0f - v) * y1 + v * y2);
}

float WorldGenerator::perlinFractal2D(float x, float y, int octaves, float persistence, float frequency) const
{
	float total = 0.0f;
	float maxAmplitude = 0.0f;
	float amplitude = 1.0f;

	for(int i=0; i<octaves; i++){
		float noiseVal = perlin2D(x * frequency, y * frequency);
		total += noiseVal * amplitude;
		maxAmplitude += amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}
	return total / maxAmplitude; // normalisé ~[-1..1]
}

// ====================================
//    Génération du monde (en chunks)
// ====================================
std::vector<std::shared_ptr<World::Chunk>> WorldGenerator::generateWorld(const Atlas& atlas)
{
	std::vector<std::shared_ptr<World::Chunk>> chunks;
	// Par exemple, 4 chunks en X, 4 chunks en Z => monde 64×64 (si CHUNK_SIZE=16)
	int numChunksX = 10;
	int numChunksZ = 10;

	for(int cx=0; cx<numChunksX; cx++){
		for(int cz=0; cz<numChunksZ; cz++){
			auto chunk = std::make_shared<World::Chunk>(cx, cz);
			chunks.push_back(chunk);
		}
	}

	// Paramètres terrain
	int   octaves     = 4;
	float persistence = 0.5f;
	float freq        = 0.01f; 
	float maxHeight   = 20.0f;   // altitude maximum
	int   baseHeight  = 10;      // altitude moyenne
	float waterLevel  = 18.0f;   // altitude de l'eau

	// Remplir chaque chunk
	for(auto &chunk : chunks){
		int cx = chunk->getChunkX();
		int cz = chunk->getChunkZ();

		// Pour chaque case [0..CHUNK_SIZE-1]
		for(int x=0; x<World::CHUNK_SIZE; x++){
			for(int z=0; z<World::CHUNK_SIZE; z++){
				// Coordonnées globales en blocs
				int worldX = cx*World::CHUNK_SIZE + x;
				int worldZ = cz*World::CHUNK_SIZE + z;

				// Bruit perlin fractal
				float noiseVal = perlinFractal2D(worldX, worldZ, octaves, persistence, freq);
				// ~entre -1 et +1
				noiseVal = (noiseVal + 1.0f)*0.5f; // => [0..1]
				// Hauteur finale
				float heightF = noiseVal*maxHeight + baseHeight; 
				int h = (int)floor(heightF);

				// On limite
				if(h < 0) h=0; 
				if(h >= World::CHUNK_HEIGHT) h = World::CHUNK_HEIGHT-1;

				// Remplir y=0..h
				for(int y=0; y<=h; y++){
					// Pour être un peu plus varié : 
					// - la surface : Grass
					// - sous la surface (3 blocs) : Dirt
					// - en dessous : Stone
					if(y == h) {
						// surface
						chunk->setBlock(x,y,z, BlockType::Grass);
					} else if(y >= h-3) {
						// 3 couches de Dirt
						chunk->setBlock(x,y,z, BlockType::Dirt);
					} else {
						// stone en profondeur
						chunk->setBlock(x,y,z, BlockType::Stone);
					}
				}

				// eau si h < waterLevel
				if(h < (int)waterLevel) {
					for(int wy=h+1; wy<=(int)waterLevel; wy++){
						if(wy < World::CHUNK_HEIGHT)
							chunk->setBlock(x,wy,z, BlockType::Water);
					}
				}
			}
		}

		// Générer le mesh du chunk
		chunk->generateMesh(atlas);
	}

	std::cout << "[WorldGenerator] Génération terminée : " 
			  << chunks.size() << " chunks." << std::endl;
	return chunks;
}

} // namespace Game
