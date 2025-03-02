#include <cmath>
#include <cstdlib>
#include <cstring>
#include <array>
#include <algorithm>

namespace {

// Constantes pour la transformation
constexpr float F2 = 0.3660254037844386f;
constexpr float G2 = 0.21132486540518713f;
constexpr float F3 = 1.0f / 3.0f;
constexpr float G3 = 1.0f / 6.0f;

// Fonctions utilitaires

// Affecte trois valeurs dans un tableau de float
inline void assign3(float a[3], float v0, float v1, float v2) {
	a[0] = v0; a[1] = v1; a[2] = v2;
}

// Surcharge pour affecter trois valeurs dans un tableau d'int
inline void assign3(int a[3], int v0, int v1, int v2) {
	a[0] = v0; a[1] = v1; a[2] = v2;
}

// Produit scalaire de deux tableaux à 3 composantes
inline float dot3(const float v1[3], const float v2[3]) {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

// Vecteur de gradients en 3D (16 directions)
constexpr std::array<std::array<float, 3>, 16> GRAD3 = {{
	{{ 1,  1,  0}}, {{-1,  1,  0}}, {{ 1, -1,  0}}, {{-1, -1,  0}},
	{{ 1,  0,  1}}, {{-1,  0,  1}}, {{ 1,  0, -1}}, {{-1,  0, -1}},
	{{ 0,  1,  1}}, {{ 0, -1,  1}}, {{ 0,  1, -1}}, {{ 0, -1, -1}},
	{{ 1,  0, -1}}, {{-1,  0, -1}}, {{ 0, -1,  1}}, {{ 0,  1,  1}}
}};

// Tableau de permutation de 512 éléments (les 256 premiers suivis d'une copie)
std::array<unsigned char, 512> PERM = {{
	151, 160, 137,  91,  90,  15, 131,  13,
	201,  95,  96,  53, 194, 233,   7, 225,
	140,  36, 103,  30,  69, 142,   8,  99,
	 37, 240,  21,  10,  23, 190,   6, 148,
	247, 120, 234,  75,   0,  26, 197,  62,
	 94, 252, 219, 203, 117,  35,  11,  32,
	 57, 177,  33,  88, 237, 149,  56,  87,
	174,  20, 125, 136, 171, 168,  68, 175,
	 74, 165,  71, 134, 139,  48,  27, 166,
	 77, 146, 158, 231,  83, 111, 229, 122,
	 60, 211, 133, 230, 220, 105,  92,  41,
	 55,  46, 245,  40, 244, 102, 143,  54,
	 65,  25,  63, 161,   1, 216,  80,  73,
	209,  76, 132, 187, 208,  89,  18, 169,
	200, 196, 135, 130, 116, 188, 159,  86,
	164, 100, 109, 198, 173, 186,   3,  64,
	 52, 217, 226, 250, 124, 123,   5, 202,
	 38, 147, 118, 126, 255,  82,  85, 212,
	207, 206,  59, 227,  47,  16,  58,  17,
	182, 189,  28,  42, 223, 183, 170, 213,
	119, 248, 152,   2,  44, 154, 163,  70,
	221, 153, 101, 155, 167,  43, 172,   9,
	129,  22,  39, 253,  19,  98, 108, 110,
	 79, 113, 224, 232, 178, 185, 112, 104,
	218, 246,  97, 228, 251,  34, 242, 193,
	238, 210, 144,  12, 191, 179, 162, 241,
	 81,  51, 145, 235, 249,  14, 239, 107,
	 49, 192, 214,  31, 181, 199, 106, 157,
	184,  84, 204, 176, 115, 121,  50,  45,
	127,   4, 150, 254, 138, 236, 205,  93,
	222, 114,  67,  29,  24,  72, 243, 141,
	128, 195,  78,  66, 215,  61, 156, 180,
	// Répétition des 256 premiers éléments
	151, 160, 137,  91,  90,  15, 131,  13,
	201,  95,  96,  53, 194, 233,   7, 225,
	140,  36, 103,  30,  69, 142,   8,  99,
	 37, 240,  21,  10,  23, 190,   6, 148,
	247, 120, 234,  75,   0,  26, 197,  62,
	 94, 252, 219, 203, 117,  35,  11,  32,
	 57, 177,  33,  88, 237, 149,  56,  87,
	174,  20, 125, 136, 171, 168,  68, 175,
	 74, 165,  71, 134, 139,  48,  27, 166,
	 77, 146, 158, 231,  83, 111, 229, 122,
	 60, 211, 133, 230, 220, 105,  92,  41,
	 55,  46, 245,  40, 244, 102, 143,  54,
	 65,  25,  63, 161,   1, 216,  80,  73,
	209,  76, 132, 187, 208,  89,  18, 169,
	200, 196, 135, 130, 116, 188, 159,  86,
	164, 100, 109, 198, 173, 186,   3,  64,
	 52, 217, 226, 250, 124, 123,   5, 202,
	 38, 147, 118, 126, 255,  82,  85, 212,
	207, 206,  59, 227,  47,  16,  58,  17,
	182, 189,  28,  42, 223, 183, 170, 213,
	119, 248, 152,   2,  44, 154, 163,  70,
	221, 153, 101, 155, 167,  43, 172,   9,
	129,  22,  39, 253,  19,  98, 108, 110,
	 79, 113, 224, 232, 178, 185, 112, 104,
	218, 246,  97, 228, 251,  34, 242, 193,
	238, 210, 144,  12, 191, 179, 162, 241,
	 81,  51, 145, 235, 249,  14, 239, 107,
	 49, 192, 214,  31, 181, 199, 106, 157,
	184,  84, 204, 176, 115, 121,  50,  45,
	127,   4, 150, 254, 138, 236, 205,  93,
	222, 114,  67,  29,  24,  72, 243, 141,
	128, 195,  78,  66, 215,  61, 156, 180
}};
	
} // namespace anonyme

namespace Noise {

// Fonction de seed
void seed(unsigned int x) {
	std::srand(x);
	for (int i = 0; i < 256; i++) {
		PERM[i] = static_cast<unsigned char>(i);
	}
	for (int i = 255; i > 0; i--) {
		int j;
		int n = i + 1;
		while (n <= (j = std::rand() / (RAND_MAX / n)));
		std::swap(PERM[i], PERM[j]);
	}
	// Recopie des 256 premiers éléments dans la seconde moitié
	std::memcpy(PERM.data() + 256, PERM.data(), 256 * sizeof(unsigned char));
}

// Fonction de bruit 2D
float noise2(float x, float y) {
	int i1, j1, I, J;
	float s = (x + y) * F2;
	float i = std::floor(x + s);
	float j = std::floor(y + s);
	float t = (i + j) * G2;

	float xx[3], yy[3], f[3];
	float noise[3] = {0.0f, 0.0f, 0.0f};
	int g[3];

	xx[0] = x - (i - t);
	yy[0] = y - (j - t);

	i1 = (xx[0] > yy[0]) ? 1 : 0;
	j1 = (xx[0] <= yy[0]) ? 1 : 0;

	xx[1] = xx[0] - i1 + G2;
	yy[1] = yy[0] - j1 + G2;
	xx[2] = xx[0] - 1.0f + 2.0f * G2;
	yy[2] = yy[0] - 1.0f + 2.0f * G2;

	I = static_cast<int>(i) & 255;
	J = static_cast<int>(j) & 255;
	g[0] = PERM[I + PERM[J]] % 12;
	g[1] = PERM[I + i1 + PERM[J + j1]] % 12;
	g[2] = PERM[I + 1 + PERM[J + 1]] % 12;

	for (int c = 0; c < 3; c++) {
		f[c] = 0.5f - xx[c] * xx[c] - yy[c] * yy[c];
	}
	
	for (int c = 0; c < 3; c++) {
		if (f[c] > 0.0f) {
			float f4 = f[c] * f[c] * f[c] * f[c];
			noise[c] = f4 * (GRAD3[g[c]][0] * xx[c] + GRAD3[g[c]][1] * yy[c]);
		}
	}
	
	return (noise[0] + noise[1] + noise[2]) * 70.0f;
}

// Fonction de bruit 3D
float noise3(float x, float y, float z) {
	int o1[3], o2[3], g[4], I, J, K;
	float f[4], noise[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float s = (x + y + z) * F3;
	float i = std::floor(x + s);
	float j = std::floor(y + s);
	float k = std::floor(z + s);
	float t = (i + j + k) * G3;

	float pos[4][3];

	pos[0][0] = x - (i - t);
	pos[0][1] = y - (j - t);
	pos[0][2] = z - (k - t);

	if (pos[0][0] >= pos[0][1]) {
		if (pos[0][1] >= pos[0][2]) {
			assign3(o1, 1, 0, 0);
			assign3(o2, 1, 1, 0);
		} else if (pos[0][0] >= pos[0][2]) {
			assign3(o1, 1, 0, 0);
			assign3(o2, 1, 0, 1);
		} else {
			assign3(o1, 0, 0, 1);
			assign3(o2, 1, 0, 1);
		}
	} else {
		if (pos[0][1] < pos[0][2]) {
			assign3(o1, 0, 0, 1);
			assign3(o2, 0, 1, 1);
		} else if (pos[0][0] < pos[0][2]) {
			assign3(o1, 0, 1, 0);
			assign3(o2, 0, 1, 1);
		} else {
			assign3(o1, 0, 1, 0);
			assign3(o2, 1, 1, 0);
		}
	}
	
	for (int c = 0; c < 3; c++) {
		pos[1][c] = pos[0][c] - o1[c] + G3;
		pos[2][c] = pos[0][c] - o2[c] + 2.0f * G3;
		pos[3][c] = pos[0][c] - 1.0f + 3.0f * G3;
	}

	I = static_cast<int>(i) & 255;
	J = static_cast<int>(j) & 255;
	K = static_cast<int>(k) & 255;
	g[0] = PERM[I + PERM[J + PERM[K]]] % 12;
	g[1] = PERM[I + o1[0] + PERM[J + o1[1] + PERM[o1[2] + K]]] % 12;
	g[2] = PERM[I + o2[0] + PERM[J + o2[1] + PERM[o2[2] + K]]] % 12;
	g[3] = PERM[I + 1 + PERM[J + 1 + PERM[K + 1]]] % 12; 

	for (int c = 0; c < 4; c++) {
		f[c] = 0.6f - pos[c][0] * pos[c][0]
				   - pos[c][1] * pos[c][1]
				   - pos[c][2] * pos[c][2];
	}
	
	for (int c = 0; c < 4; c++) {
		if (f[c] > 0.0f) {
			// Utilisation de .data() pour obtenir un pointeur vers le tableau interne du std::array
			noise[c] = f[c] * f[c] * f[c] * f[c] * dot3(pos[c], GRAD3[g[c]].data());
		}
	}
	
	return (noise[0] + noise[1] + noise[2] + noise[3]) * 32.0f;
}

// Bruit fractal 2D (simplex2)
float simplex2(float x, float y, int octaves, float persistence, float lacunarity) {
	float freq = 1.0f;
	float amp = 1.0f;
	float maxAmp = 1.0f;
	float total = noise2(x, y);
	for (int i = 1; i < octaves; i++) {
		freq *= lacunarity;
		amp *= persistence;
		maxAmp += amp;
		total += noise2(x * freq, y * freq) * amp;
	}
	return (1.0f + total / maxAmp) / 2.0f;
}

// Bruit fractal 3D (simplex3)
float simplex3(float x, float y, float z, int octaves, float persistence, float lacunarity) {
	float freq = 1.0f;
	float amp = 1.0f;
	float maxAmp = 1.0f;
	float total = noise3(x, y, z);
	for (int i = 1; i < octaves; i++) {
		freq *= lacunarity;
		amp *= persistence;
		maxAmp += amp;
		total += noise3(x * freq, y * freq, z * freq) * amp;
	}
	return (1.0f + total / maxAmp) / 2.0f;
}

} // namespace Noise
