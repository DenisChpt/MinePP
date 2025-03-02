#ifndef UTILS_HPP
#define UTILS_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Matrix.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cerrno>

// Définition de fonctions inline pour remplacer quelques macros
namespace Utils {

	// Conversion d'angles
	inline constexpr float degrees(float radians) {
		return radians * 180.0f / PI;
	}

	inline constexpr float radians(float degrees) {
		return degrees * PI / 180.0f;
	}

	// Fonctions génériques
	template<typename T>
	inline T abs(T x) {
		return (x < 0) ? -x : x;
	}

	template<typename T>
	inline T min_val(T a, T b) {
		return (a < b) ? a : b;
	}

	template<typename T>
	inline T max_val(T a, T b) {
		return (a > b) ? a : b;
	}

	template<typename T>
	inline int sign(T x) {
		return (x > 0) - (x < 0);
	}

	// Structure pour le calcul du FPS
	struct FPS {
		unsigned int fps;
		unsigned int frames;
		double since;
	};

	// Fonctions utilitaires aléatoires
	int rand_int(int n);
	double rand_double();
	void update_fps(FPS &fps);

	// Fonctions OpenGL pour la gestion des buffers
	GLuint gen_buffer(GLsizei size, const GLfloat *data);
	void del_buffer(GLuint buffer);
	// Alloue un tableau pour stocker les faces (6 * components par face)
	GLfloat *malloc_faces(int components, int faces);
	// Génère un buffer à partir des faces et libère la mémoire allouée
	GLuint gen_faces(int components, int faces, GLfloat *data);

	// Fonctions de gestion des shaders
	GLuint make_shader(GLenum type, const char *source);
	GLuint load_shader(GLenum type, const char *path);
	GLuint make_program(GLuint shader1, GLuint shader2);
	GLuint load_program(const char *path1, const char *path2);

	// Chargement d'une texture PNG dans la texture courante OpenGL
	void load_png_texture(const char *file_name);

	// Fonction de tokenisation d'une chaîne (version C)
	char *tokenize(char *str, const char *delim, char **key);
	// Calcul de la largeur d'un caractère (selon une table fixe ASCII)
	int char_width(char input);
	// Calcul de la largeur d'une chaîne de caractères
	int string_width(const char *input);
	// Réalise un wrapping de texte : renvoie le nombre de lignes générées dans output
	int wrap(const char *input, int max_width, char *output, int max_length);

} // namespace Utils

#endif // UTILS_HPP
