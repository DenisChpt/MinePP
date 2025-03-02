#ifndef RENDER_MANAGER_HPP
#define RENDER_MANAGER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Config.hpp"
#include "Utils.hpp"
#include "Cube.hpp"
#include "World.hpp"
#include "Map.hpp"
#include "Sign.hpp"
#include "InputManager.hpp"

// Structure regroupant les attributs shader utilisés pour le rendu.
struct Attrib {
	GLuint program = 0;
	GLuint position = 0;
	GLuint normal = 0;
	GLuint uv = 0;
	GLuint matrix = 0;
	GLuint sampler = 0;
	GLuint camera = 0;
	GLuint timer = 0;
	GLuint extra1 = 0;
	GLuint extra2 = 0;
	GLuint extra3 = 0;
	GLuint extra4 = 0;
};

// Structures minimales pour Chunk et Player, utilisées ici pour le rendu.
// Dans une architecture complète ces types seraient définis dans leur module respectif.
struct Chunk {
	World::Map map;
	World::Map lights;
	Utils::SignList signs;
	int p = 0, q = 0;
	int faces = 0;
	int sign_faces = 0;
	GLuint buffer = 0;
	GLuint sign_buffer = 0;
};

struct Player {
	int id = 0;
	char name[32] = {0}; // MAX_NAME_LENGTH
	State state;
	GLuint buffer = 0;
};

namespace RenderManager {

// --- Fonctions d'initialisation du rendu ---
/// Initialise l'état OpenGL (viewport, culling, depth test, etc.)
void initGL(const Config &config, int width, int height);

// --- Fonctions de génération de buffers ---
/// Génère le buffer pour le ciel.
GLuint genSkyBuffer();
/// Génère le buffer pour le crosshair (en 2D).
GLuint genCrosshairBuffer(int width, int height, int scale);
/// Génère le buffer pour le joueur à partir de son état.
GLuint genPlayerBuffer(const State &state);
/// Génère le buffer d'un cube ou d'un bloc.
GLuint genCubeBuffer(float x, float y, float z, float n, int w);
/// Génère le buffer d'une plante.
GLuint genPlantBuffer(float x, float y, float z, float n, int w);
/// Génère le buffer pour du texte.
GLuint genTextBuffer(float x, float y, float n, const char *text);

// --- Fonctions de dessin primitives ---
/// Dessine des triangles 3D avec occlusion ambiante (AO).
void drawTriangles3D_AO(const Attrib &attrib, GLuint buffer, int count);
/// Dessine des triangles 3D destinés au texte.
void drawTriangles3D_Text(const Attrib &attrib, GLuint buffer, int count);
/// Dessine des triangles 3D généraux.
void drawTriangles3D(const Attrib &attrib, GLuint buffer, int count);
/// Dessine des triangles 2D.
void drawTriangles2D(const Attrib &attrib, GLuint buffer, int count);
/// Dessine des lignes.
void drawLines(const Attrib &attrib, GLuint buffer, int components, int count);

// --- Fonctions de rendu d'objets ---
/// Rend le ciel.
void renderSky(const Attrib &attrib, const State &cameraState, GLuint skyBuffer,
			   int width, int height, int renderRadius, float fov, int ortho);
/// Rend les chunks et retourne le nombre total de faces.
int renderChunks(const Attrib &attrib, const State &cameraState,
				 Chunk *chunks, int chunkCount, int renderRadius,
				 int width, int height, float fov, int ortho);
/// Rend les joueurs.
void renderPlayers(const Attrib &attrib, const State &cameraState,
				   Player *players, int playerCount,
				   int width, int height, float fov, int ortho);
/// Rend le crosshair.
void renderCrosshairs(const Attrib &attrib, int width, int height, int scale);
/// Rend l’item sélectionné.
void renderItem(const Attrib &attrib, int width, int height, int scale, int itemIndex);
/// Rend du texte.
void renderText(const Attrib &attrib, int justify, float x, float y, float n,
				const char *text, int width, int height);
/// Rend les panneaux (signs).
void renderSigns(const Attrib &attrib, const State &cameraState,
				 Chunk *chunks, int chunkCount, int signRadius,
				 int width, int height, float fov, int ortho);
/// Rend le wireframe autour d’un objet.
void renderWireframe(const Attrib &attrib, const State &cameraState,
					 Chunk *chunks, int chunkCount, int renderRadius,
					 int width, int height, float fov, int ortho);

} // namespace RenderManager

#endif // RENDER_MANAGER_HPP
