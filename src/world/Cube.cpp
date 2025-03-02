#include "Cube.hpp"
#include "Matrix.hpp"
#include "Item.hpp"
#include "Utils.hpp"

namespace Cube {

// --- Fonction interne : makeCubeFaces ---
// Les tableaux statiques définissent les positions, normales, coordonnées UV et indices pour chaque face.
void makeCubeFaces(
	float *data, const float ao[6][4], const float light[6][4],
	int left, int right, int top, int bottom, int front, int back,
	int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
	float x, float y, float z, float n)
{
	static const float positions[6][4][3] = {
		{ {-1, -1, -1}, {-1, -1, +1}, {-1, +1, -1}, {-1, +1, +1} },
		{ {+1, -1, -1}, {+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1} },
		{ {-1, +1, -1}, {-1, +1, +1}, {+1, +1, -1}, {+1, +1, +1} },
		{ {-1, -1, -1}, {-1, -1, +1}, {+1, -1, -1}, {+1, -1, +1} },
		{ {-1, -1, -1}, {-1, +1, -1}, {+1, -1, -1}, {+1, +1, -1} },
		{ {-1, -1, +1}, {-1, +1, +1}, {+1, -1, +1}, {+1, +1, +1} }
	};
	static const float normals[6][3] = {
		{-1, 0, 0},
		{+1, 0, 0},
		{0, +1, 0},
		{0, -1, 0},
		{0, 0, -1},
		{0, 0, +1}
	};
	static const float uvs[6][4][2] = {
		{ {0, 0}, {1, 0}, {0, 1}, {1, 1} },
		{ {1, 0}, {0, 0}, {1, 1}, {0, 1} },
		{ {0, 1}, {0, 0}, {1, 1}, {1, 0} },
		{ {0, 0}, {0, 1}, {1, 0}, {1, 1} },
		{ {0, 0}, {0, 1}, {1, 0}, {1, 1} },
		{ {1, 0}, {1, 1}, {0, 0}, {0, 1} }
	};
	static const int indices[6][6] = {
		{0, 3, 2, 0, 1, 3},
		{0, 3, 1, 0, 2, 3},
		{0, 3, 2, 0, 1, 3},
		{0, 3, 1, 0, 2, 3},
		{0, 3, 2, 0, 1, 3},
		{0, 3, 1, 0, 2, 3}
	};
	static const int flipped[6][6] = {
		{0, 1, 2, 1, 3, 2},
		{0, 2, 1, 2, 3, 1},
		{0, 1, 2, 1, 3, 2},
		{0, 2, 1, 2, 3, 1},
		{0, 1, 2, 1, 3, 2},
		{0, 2, 1, 2, 3, 1}
	};

	const float s = 0.0625f;
	// Ajustement pour éviter les artefacts (petite marge).
	const float aMargin = 1.0f / 2048.0f;
	const float bMargin = s - 1.0f / 2048.0f;
	int faces[6] = { left, right, top, bottom, front, back };
	int tiles[6] = { wleft, wright, wtop, wbottom, wfront, wback };

	float *d = data;
	for (int i = 0; i < 6; i++) {
		if (faces[i] == 0)
			continue;
		float du = (tiles[i] % 16) * s;
		float dv = (tiles[i] / 16) * s;
		// Décision de flip en fonction des valeurs d'occlusion.
		int flip = (ao[i][0] + ao[i][3] > ao[i][1] + ao[i][2]) ? 1 : 0;
		for (int v = 0; v < 6; v++) {
			int j = flip ? flipped[i][v] : indices[i][v];
			*(d++) = x + n * positions[i][j][0];
			*(d++) = y + n * positions[i][j][1];
			*(d++) = z + n * positions[i][j][2];
			*(d++) = normals[i][0];
			*(d++) = normals[i][1];
			*(d++) = normals[i][2];
			*(d++) = du + ((uvs[i][j][0] != 0) ? bMargin : aMargin);
			*(d++) = dv + ((uvs[i][j][1] != 0) ? bMargin : aMargin);
			*(d++) = ao[i][j];
			*(d++) = light[i][j];
		}
	}
}

void makeCube(
	float *data, const float ao[6][4], const float light[6][4],
	int left, int right, int top, int bottom, int front, int back,
	float x, float y, float z, float n, int w)
{
	// Récupération des indices de texture pour le type w via World::blocks
	int wleft   = World::blocks[w][0];
	int wright  = World::blocks[w][1];
	int wtop    = World::blocks[w][2];
	int wbottom = World::blocks[w][3];
	int wfront  = World::blocks[w][4];
	int wback   = World::blocks[w][5];
	makeCubeFaces(data, ao, light,
				  left, right, top, bottom, front, back,
				  wleft, wright, wtop, wbottom, wfront, wback,
				  x, y, z, n);
}

void makePlant(
	float *data, float ao, float light,
	float px, float py, float pz, float n, int w, float rotation)
{
	// Tableaux définissant la géométrie de la plante.
	static const float positions[4][4][3] = {
		{ { 0, -1, -1}, { 0, -1, +1}, { 0, +1, -1}, { 0, +1, +1} },
		{ { 0, -1, -1}, { 0, -1, +1}, { 0, +1, -1}, { 0, +1, +1} },
		{ {-1, -1,  0}, {-1, +1,  0}, {+1, -1,  0}, {+1, +1,  0} },
		{ {-1, -1,  0}, {-1, +1,  0}, {+1, -1,  0}, {+1, +1,  0} }
	};
	static const float normals[4][3] = {
		{-1, 0, 0},
		{+1, 0, 0},
		{0, 0, -1},
		{0, 0, +1}
	};
	static const float uvs[4][4][2] = {
		{ {0, 0}, {1, 0}, {0, 1}, {1, 1} },
		{ {1, 0}, {0, 0}, {1, 1}, {0, 1} },
		{ {0, 0}, {0, 1}, {1, 0}, {1, 1} },
		{ {1, 0}, {1, 1}, {0, 0}, {0, 1} }
	};
	static const int indices[4][6] = {
		{0, 3, 2, 0, 1, 3},
		{0, 3, 1, 0, 2, 3},
		{0, 3, 2, 0, 1, 3},
		{0, 3, 1, 0, 2, 3}
	};

	float *d = data;
	const float s = 0.0625f;
	float a = 0.0f, b = s;
	// Utilise le tableau de textures pour plantes défini dans World::plants
	float du = (World::plants[w] % 16) * s;
	float dv = (World::plants[w] / 16) * s;
	for (int i = 0; i < 4; i++) {
		for (int v = 0; v < 6; v++) {
			int j = indices[i][v];
			*(d++) = n * positions[i][j][0];
			*(d++) = n * positions[i][j][1];
			*(d++) = n * positions[i][j][2];
			*(d++) = normals[i][0];
			*(d++) = normals[i][1];
			*(d++) = normals[i][2];
			*(d++) = du + ((uvs[i][j][0] != 0) ? b : a);
			*(d++) = dv + ((uvs[i][j][1] != 0) ? b : a);
			*(d++) = ao;
			*(d++) = light;
		}
	}
	// Appliquer une rotation via matrice sur les 24 sommets générés (4 faces * 6 vertices)
	float ma[16], mb[16];
	Utils::mat_identity(ma);
	// Conversion de rotation en radians (utilisation de la fonction inline 'radians')
	Utils::mat_rotate(mb, 0, 1, 0, Utils::radians(rotation));
	Utils::mat_multiply(ma, mb, ma);
	// Applique la transformation sur les normales (décalage à partir de l'offset 3, stride 10)
	Utils::mat_apply(data, ma, 24, 3, 10);
	Utils::mat_translate(mb, px, py, pz);
	Utils::mat_multiply(ma, mb, ma);
	Utils::mat_apply(data, ma, 24, 0, 10);
}

void makePlayer(
	float *data,
	float x, float y, float z, float rx, float ry)
{
	float ao[6][4] = {0};
	float light[6][4] = {
		{0.8f, 0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f, 0.8f}
	};
	// Utilise makeCubeFaces avec des indices de texture fixes pour le joueur
	makeCubeFaces(data, ao, light,
				   1, 1, 1, 1, 1, 1,
				   226, 224, 241, 209, 225, 227,
				   0, 0, 0, 0.4f);
	float ma[16], mb[16];
	Utils::mat_identity(ma);
	Utils::mat_rotate(mb, 0, 1, 0, rx);
	Utils::mat_multiply(ma, mb, ma);
	Utils::mat_rotate(mb, std::cosf(rx), 0, std::sinf(rx), -ry);
	Utils::mat_multiply(ma, mb, ma);
	Utils::mat_apply(data, ma, 36, 3, 10);
	Utils::mat_translate(mb, x, y, z);
	Utils::mat_multiply(ma, mb, ma);
	Utils::mat_apply(data, ma, 36, 0, 10);
}

void makeCubeWireframe(float *data, float x, float y, float z, float n) {
	static const float positions[8][3] = {
		{-1, -1, -1},
		{-1, -1, +1},
		{-1, +1, -1},
		{-1, +1, +1},
		{+1, -1, -1},
		{+1, -1, +1},
		{+1, +1, -1},
		{+1, +1, +1}
	};
	static const int indices[24] = {
		0, 1, 0, 2, 0, 4, 1, 3,
		1, 5, 2, 3, 2, 6, 3, 7,
		4, 5, 4, 6, 5, 7, 6, 7
	};
	float *d = data;
	for (int i = 0; i < 24; i++) {
		int j = indices[i];
		*(d++) = x + n * positions[j][0];
		*(d++) = y + n * positions[j][1];
		*(d++) = z + n * positions[j][2];
	}
}

void makeCharacter(
	float *data,
	float x, float y, float n, float m, char c)
{
	float *d = data;
	const float s = 0.0625f;
	float a = s, b = s * 2;
	int w = c - 32;
	float du = (w % 16) * a;
	float dv = 1 - (w / 16) * b - b;
	*(d++) = x - n; *(d++) = y - m;
	*(d++) = du;    *(d++) = dv;
	*(d++) = x + n; *(d++) = y - m;
	*(d++) = du + a;*(d++) = dv;
	*(d++) = x + n; *(d++) = y + m;
	*(d++) = du + a;*(d++) = dv + b;
	*(d++) = x - n; *(d++) = y - m;
	*(d++) = du;    *(d++) = dv;
	*(d++) = x + n; *(d++) = y + m;
	*(d++) = du + a;*(d++) = dv + b;
	*(d++) = x - n; *(d++) = y + m;
	*(d++) = du;    *(d++) = dv + b;
}

void makeCharacter3D(
	float *data, float x, float y, float z, float n, int face, char c)
{
	// Pour simplifier, on définit ici les positions, UV et offsets pour la face demandée.
	static const float positions[8][6][3] = {
		{ {0, -2, -1}, {0, +2, +1}, {0, +2, -1},
		  {0, -2, -1}, {0, -2, +1}, {0, +2, +1} },
		{ {0, -2, -1}, {0, +2, +1}, {0, -2, +1},
		  {0, -2, -1}, {0, +2, -1}, {0, +2, +1} },
		{ {-1, -2, 0}, {+1, +2, 0}, {+1, -2, 0},
		  {-1, -2, 0}, {-1, +2, 0}, {+1, +2, 0} },
		{ {-1, -2, 0}, {+1, -2, 0}, {+1, +2, 0},
		  {-1, -2, 0}, {+1, +2, 0}, {-1, +2, 0} },
		{ {-1, 0, +2}, {+1, 0, +2}, {+1, 0, -2},
		  {-1, 0, +2}, {+1, 0, -2}, {-1, 0, -2} },
		{ {-2, 0, +1}, {+2, 0, -1}, {-2, 0, -1},
		  {-2, 0, +1}, {+2, 0, +1}, {+2, 0, -1} },
		{ {+1, 0, +2}, {-1, 0, -2}, {-1, 0, +2},
		  {+1, 0, +2}, {+1, 0, -2}, {-1, 0, -2} },
		{ {+2, 0, -1}, {-2, 0, +1}, {+2, 0, +1},
		  {+2, 0, -1}, {-2, 0, -1}, {-2, 0, +1} }
	};
	static const float uvs[8][6][2] = {
		{ {0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1} },
		{ {1, 0}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1} },
		{ {1, 0}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1} },
		{ {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} },
		{ {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} },
		{ {0, 1}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0} },
		{ {0, 1}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0} },
		{ {0, 1}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0} }
	};
	static const float offsets[8][3] = {
		{-1, 0, 0}, {+1, 0, 0}, {0, 0, -1}, {0, 0, +1},
		{0, +1, 0}, {0, +1, 0}, {0, +1, 0}, {0, +1, 0}
	};

	float *d = data;
	const float s = 0.0625f;
	const float pu = s / 5;
	const float pv = s / 2.5f;
	float u1 = pu, v1 = pv;
	float u2 = s - pu, v2 = s * 2 - pv;
	const float pVal = 0.5f;
	int w = c - 32;
	float du = (w % 16) * s;
	float dv = 1 - ((w / 16) + 1) * s * 2;
	// Décale la position selon la face
	x += pVal * offsets[face][0];
	y += pVal * offsets[face][1];
	z += pVal * offsets[face][2];
	for (int i = 0; i < 6; i++) {
		*(d++) = x + n * positions[face][i][0];
		*(d++) = y + n * positions[face][i][1];
		*(d++) = z + n * positions[face][i][2];
		*(d++) = du + ((uvs[face][i][0] != 0) ? u2 : u1);
		*(d++) = dv + ((uvs[face][i][1] != 0) ? v2 : v1);
	}
}

// Fonction récursive pour générer une sphère en subdivisant un triangle.
// Les paramètres 'a', 'b', 'c' sont les sommets initiaux et 'ta','tb','tc' leurs coordonnées UV.
int _makeSphere(
	float *data, float r, int detail,
	const float *a, const float *b, const float *c,
	const float *ta, const float *tb, const float *tc)
{
	if (detail == 0) {
		float *d = data;
		*(d++) = a[0] * r; *(d++) = a[1] * r; *(d++) = a[2] * r;
		*(d++) = a[0];    *(d++) = a[1];    *(d++) = a[2];
		*(d++) = ta[0];   *(d++) = ta[1];

		*(d++) = b[0] * r; *(d++) = b[1] * r; *(d++) = b[2] * r;
		*(d++) = b[0];    *(d++) = b[1];    *(d++) = b[2];
		*(d++) = tb[0];   *(d++) = tb[1];

		*(d++) = c[0] * r; *(d++) = c[1] * r; *(d++) = c[2] * r;
		*(d++) = c[0];    *(d++) = c[1];    *(d++) = c[2];
		*(d++) = tc[0];   *(d++) = tc[1];
		return 1;
	} else {
		float ab[3], ac[3], bc[3];
		for (int i = 0; i < 3; i++) {
			ab[i] = (a[i] + b[i]) / 2.0f;
			ac[i] = (a[i] + c[i]) / 2.0f;
			bc[i] = (b[i] + c[i]) / 2.0f;
		}
		// Normalisation des vecteurs pour les projeter sur la sphère.
		Utils::normalize(&ab[0], &ab[1], &ab[2]);
		Utils::normalize(&ac[0], &ac[1], &ac[2]);
		Utils::normalize(&bc[0], &bc[1], &bc[2]);
		float tab[2] = {0, 1 - std::acos(ab[1]) / PI};
		float tac[2] = {0, 1 - std::acos(ac[1]) / PI};
		float tbc[2] = {0, 1 - std::acos(bc[1]) / PI};
		int total = 0;
		int n;
		n = _makeSphere(data, r, detail - 1, a, ab, ac, ta, tab, tac);
		total += n; data += n * 24;
		n = _makeSphere(data, r, detail - 1, b, bc, ab, tb, tbc, tab);
		total += n; data += n * 24;
		n = _makeSphere(data, r, detail - 1, c, ac, bc, tc, tac, tbc);
		total += n; data += n * 24;
		n = _makeSphere(data, r, detail - 1, ab, bc, ac, tab, tbc, tac);
		total += n; data += n * 24;
		return total;
	}
}

void makeSphere(float *data, float r, int detail) {
	// Les 8 triangles initiaux de l'octaèdre qui sera subdivisé.
	static int indices[8][3] = {
		{4, 3, 0}, {1, 4, 0},
		{3, 4, 5}, {4, 1, 5},
		{0, 3, 2}, {0, 2, 1},
		{5, 2, 3}, {5, 1, 2}
	};
	static float positions[6][3] = {
		{ 0, 0,-1}, { 1, 0, 0},
		{ 0,-1, 0}, {-1, 0, 0},
		{ 0, 1, 0}, { 0, 0, 1}
	};
	static float uvs[6][2] = {
		{0, 0.5f}, {0, 0.5f},
		{0, 0},   {0, 0.5f},
		{0, 1},   {0, 0.5f}
	};
	int total = 0;
	for (int i = 0; i < 8; i++) {
		int n = _makeSphere(
			data, r, detail,
			positions[indices[i][0]],
			positions[indices[i][1]],
			positions[indices[i][2]],
			uvs[indices[i][0]], uvs[indices[i][1]], uvs[indices[i][2]]);
		total += n;
		data += n * 24;
	}
}

} // namespace Cube
