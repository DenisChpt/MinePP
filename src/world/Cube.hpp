#ifndef CUBE_HPP
#define CUBE_HPP

#include <cstddef>
#include <cmath>

// Dépendances : fonctions de matrices, items et utilitaires.
#include "Matrix.hpp"   // Contient Utils::mat_identity, mat_rotate, mat_multiply, mat_translate, mat_apply, normalize, etc.
#include "Item.hpp"     // Contient les constantes de blocs et de plantes dans le namespace World.

namespace Cube {

	// Génère les faces d'un cube dans le tableau 'data'.
	// - ao et light : tableaux 6x4 pour l'occlusion ambiante et l'éclairage.
	// - left, right, top, bottom, front, back : flags d'activation de chaque face.
	// - wleft, wright, wtop, wbottom, wfront, wback : indices de texture à utiliser pour chaque face.
	// - (x,y,z) : position du cube, n : taille (demi-largeur).
	void makeCubeFaces(
		float *data, const float ao[6][4], const float light[6][4],
		int left, int right, int top, int bottom, int front, int back,
		int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
		float x, float y, float z, float n);

	// Construit un cube complet en déterminant automatiquement les indices de texture
	// à partir du type 'w' (via World::blocks).
	void makeCube(
		float *data, const float ao[6][4], const float light[6][4],
		int left, int right, int top, int bottom, int front, int back,
		float x, float y, float z, float n, int w);

	// Construit la géométrie d'une plante.
	// - ao, light : intensités d'occlusion et d'éclairage (valeurs uniformes pour la face).
	// - (px,py,pz) : position, n : taille, w : type (pour récupérer la texture via World::plants),
	//   rotation en degrés.
	void makePlant(
		float *data, float ao, float light,
		float px, float py, float pz, float n, int w, float rotation);

	// Construit la géométrie du joueur (cube avec rotations appliquées).
	void makePlayer(
		float *data,
		float x, float y, float z, float rx, float ry);

	// Construit la géométrie d'un cube en wireframe.
	void makeCubeWireframe(
		float *data, float x, float y, float z, float n);

	// Construit la géométrie d'un caractère 2D.
	void makeCharacter(
		float *data,
		float x, float y, float n, float m, char c);

	// Construit la géométrie d'un caractère en 3D sur une face donnée.
	void makeCharacter3D(
		float *data, float x, float y, float z, float n, int face, char c);

	// Construit la géométrie d'une sphère en subdivisant des triangles.
	// La fonction remplit 'data' avec les sommets (chaque triangle fournissant 3 sommets, 24 floats par triangle).
	// Le paramètre 'detail' détermine le niveau de subdivision.
	void makeSphere(float *data, float r, int detail);

} // namespace Cube

#endif // CUBE_HPP
