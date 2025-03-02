#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cmath>

// Définition d'une constante PI robuste en C++
constexpr float PI = 3.14159265358979323846f;

namespace Utils {

// Normalise le vecteur (x, y, z)
void normalize(float* x, float* y, float* z);

// Initialise une matrice 4x4 en matrice identité
void mat_identity(float* matrix);

// Crée une matrice de translation avec dx, dy, dz
void mat_translate(float* matrix, float dx, float dy, float dz);

// Crée une matrice de rotation autour de l'axe (x, y, z) d'un angle donné (en radians)
void mat_rotate(float* matrix, float x, float y, float z, float angle);

// Multiplie un vecteur (de 4 composantes) par la matrice a et le vecteur b, stocke le résultat dans vector
void mat_vec_multiply(float* vector, float* a, float* b);

// Multiplie deux matrices 4x4 : matrix = a * b
void mat_multiply(float* matrix, float* a, float* b);

// Applique une transformation matricielle à un tableau de données (chaque élément est un vecteur 3D stocké avec un décalage)
void mat_apply(float* data, float* matrix, int count, int offset, int stride);

// Calcule les 6 plans de coupe du frustum à partir d'une matrice et d'un radius
void frustum_planes(float planes[6][4], int radius, float* matrix);

// Construit une matrice de frustum
void mat_frustum(float* matrix, float left, float right, float bottom, float top, float znear, float zfar);

// Construit une matrice de perspective
void mat_perspective(float* matrix, float fov, float aspect, float znear, float zfar);

// Construit une matrice orthographique
void mat_ortho(float* matrix, float left, float right, float bottom, float top, float near, float far);

// Configure une matrice 2D en fonction de la largeur et de la hauteur
void set_matrix_2d(float* matrix, int width, int height);

// Configure une matrice 3D avec les paramètres de vue et de projection
void set_matrix_3d(
	float* matrix, int width, int height,
	float x, float y, float z, float rx, float ry,
	float fov, int ortho, int radius);

// Configure une matrice pour l'affichage d'un item (par exemple, dans une interface)
void set_matrix_item(float* matrix, int width, int height, int scale);

} // namespace Utils

#endif // MATRIX_HPP
