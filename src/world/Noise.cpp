#include "Noise.hpp"

#include <cmath>
#include <cstdlib>
#include <algorithm> // Pour std::swap

// Constructeur : initialise la permutation avec la graine fournie.
Noise::Noise(unsigned int seed) {
	initPermutation(seed);
}

Noise::~Noise() {
}

// Initialise le tableau de permutation (de taille 512 après duplication)
void Noise::initPermutation(unsigned int seed) {
	permutation.resize(256);
	// Remplit avec les valeurs de 0 à 255.
	for (int i = 0; i < 256; ++i) {
		permutation[i] = i;
	}
	// Mélange le tableau avec la graine donnée.
	srand(seed);
	for (int i = 255; i > 0; --i) {
		int j = rand() % (i + 1);
		std::swap(permutation[i], permutation[j]);
	}
	// Duplique le tableau pour éviter les débordements dans les calculs.
	permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

// Fonction fade de Ken Perlin pour lisser les courbes d'interpolation.
double Noise::fade(double t) const {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

// Interpolation linéaire.
double Noise::lerp(double a, double b, double t) const {
	return a + t * (b - a);
}

// Calcule le gradient pour le coin du cube (contribution).
double Noise::grad(int hash, double x, double y, double z) const {
	int h = hash & 15;
	double u = h < 8 ? x : y;
	double v = h < 4 ? y : ((h == 12 || h == 14) ? x : z);
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

// Génère une valeur de Perlin Noise en 3D pour les coordonnées (x, y, z).
double Noise::perlinNoise(double x, double y, double z) const {
	// Trouve la cellule (cube) qui contient le point.
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;

	// Coordonnées relatives dans le cube.
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Calcul des courbes d'interpolation.
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash des 8 coins du cube.
	int A  = permutation[X] + Y;
	int AA = permutation[A] + Z;
	int AB = permutation[A + 1] + Z;
	int B  = permutation[X + 1] + Y;
	int BA = permutation[B] + Z;
	int BB = permutation[B + 1] + Z;

	// Interpole les contributions des 8 coins.
	double res = lerp(
		lerp(
			lerp(grad(permutation[AA], x, y, z),
				 grad(permutation[BA], x - 1, y, z), u),
			lerp(grad(permutation[AB], x, y - 1, z),
				 grad(permutation[BB], x - 1, y - 1, z), u),
			v),
		lerp(
			lerp(grad(permutation[AA + 1], x, y, z - 1),
				 grad(permutation[BA + 1], x - 1, y, z - 1), u),
			lerp(grad(permutation[AB + 1], x, y - 1, z - 1),
				 grad(permutation[BB + 1], x - 1, y - 1, z - 1), u),
			v),
		w);

	return res;
}

// Génère un bruit fractal en combinant plusieurs octaves de Perlin Noise.
double Noise::octaveNoise(double x, double y, double z, int octaves, double persistence, double frequency) const {
	double total = 0.0;
	double maxValue = 0.0;
	double amplitude = 1.0;

	for (int i = 0; i < octaves; ++i) {
		total += perlinNoise(x * frequency, y * frequency, z * frequency) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2.0;
	}
	return total / maxValue;
}
