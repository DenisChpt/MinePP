#include "Noise.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Noise::Noise(unsigned int seed) {
	initPermutation(seed);
}

Noise::~Noise() {
}

void Noise::initPermutation(unsigned int seed) {
	permutation.resize(256);
	for (int i = 0; i < 256; ++i) {
		permutation[i] = i;
	}
	srand(seed);
	for (int i = 255; i > 0; --i) {
		int j = rand() % (i + 1);
		std::swap(permutation[i], permutation[j]);
	}
	permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

double Noise::fade(double t) const {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double Noise::lerp(double a, double b, double t) const {
	return a + t * (b - a);
}

double Noise::grad(int hash, double x, double y, double z) const {
	int h = hash & 15;
	double u = (h < 8) ? x : y;
	double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
	double result = 0.0;
	if ((h & 1) == 0) { result += u; } else { result -= u; }
	if ((h & 2) == 0) { result += v; } else { result -= v; }
	return result;
}

double Noise::perlinNoise(double x, double y, double z) const {
	int X = (int)std::floor(x) & 255;
	int Y = (int)std::floor(y) & 255;
	int Z = (int)std::floor(z) & 255;

	x -= std::floor(x);
	y -= std::floor(y);
	z -= std::floor(z);

	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	int A  = permutation[X] + Y;
	int AA = permutation[A] + Z;
	int AB = permutation[A + 1] + Z;
	int B  = permutation[X + 1] + Y;
	int BA = permutation[B] + Z;
	int BB = permutation[B + 1] + Z;

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
