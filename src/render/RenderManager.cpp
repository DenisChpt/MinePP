#include "RenderManager.hpp"
#include "Utils.hpp"
#include "Cube.hpp"
#include "World.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// Définition d'une constante pour le nombre de composants par vertex dans les buffers pour les objets 3D AO.
#define FACE_COMPONENTS 10
#define FACE_VERTEX_COUNT 6

namespace RenderManager {

// --- Fonctions d'initialisation du rendu ---
void initGL(const Config &config, int width, int height) {
	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glLogicOp(GL_INVERT);
	glClearColor(0, 0, 0, 1);
}

// --- Fonctions de génération de buffers ---
GLuint genSkyBuffer() {
	// Utilisation de Cube::makeSphere pour générer la géométrie du ciel.
	float data[12288];
	Cube::makeSphere(data, 1.0f, 3);
	return Utils::gen_buffer(sizeof(data), data);
}

GLuint genCrosshairBuffer(int width, int height, int scale) {
	int x = width / 2;
	int y = height / 2;
	int p = 10 * scale;
	float data[] = {
		static_cast<float>(x), static_cast<float>(y - p),
		static_cast<float>(x), static_cast<float>(y + p),
		static_cast<float>(x - p), static_cast<float>(y),
		static_cast<float>(x + p), static_cast<float>(y)
	};
	return Utils::gen_buffer(sizeof(data), data);
}

GLuint genPlayerBuffer(const State &state) {
	// Alloue les faces pour un joueur (ex. 6 faces, 36 vertices)
	GLfloat *data = Utils::malloc_faces(10, 6);
	Cube::makeCharacter(data, state.x, state.y, state.z, state.rx, state.ry);
	GLuint buffer = Utils::gen_faces(10, 6, data);
	return buffer;
}

GLuint genCubeBuffer(float x, float y, float z, float n, int w) {
	GLfloat *data = Utils::malloc_faces(10, 6);
	float ao[6][4] = {0};
	float light[6][4] = {
		{0.5f, 0.5f, 0.5f, 0.5f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{0.5f, 0.5f, 0.5f, 0.5f}
	};
	Cube::makeCube(data, ao, light, 1, 1, 1, 1, 1, 1, x, y, z, n, w);
	GLuint buffer = Utils::gen_faces(10, 6, data);
	return buffer;
}

GLuint genPlantBuffer(float x, float y, float z, float n, int w) {
	GLfloat *data = Utils::malloc_faces(10, 4);
	float ao = 0.0f;
	float light = 1.0f;
	Cube::makePlant(data, ao, light, x, y, z, n, w, 45.0f);
	GLuint buffer = Utils::gen_faces(10, 4, data);
	return buffer;
}

GLuint genTextBuffer(float x, float y, float n, const char *text) {
	int length = static_cast<int>(strlen(text));
	GLfloat *data = Utils::malloc_faces(4, length);
	for (int i = 0; i < length; i++) {
		Cube::makeCharacter(data + i * 24, x, y, n / 2, n, text[i]);
		x += n;
	}
	GLuint buffer = Utils::gen_faces(4, length, data);
	return buffer;
}

// --- Fonctions de dessin primitives ---
static void drawTriangles3D_AO_internal(const Attrib &attrib, GLuint buffer, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib.position);
	glEnableVertexAttribArray(attrib.normal);
	glEnableVertexAttribArray(attrib.uv);
	glVertexAttribPointer(attrib.position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * FACE_COMPONENTS, reinterpret_cast<GLvoid*>(0));
	glVertexAttribPointer(attrib.normal, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * FACE_COMPONENTS, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*3));
	glVertexAttribPointer(attrib.uv, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * FACE_COMPONENTS, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*6));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib.position);
	glDisableVertexAttribArray(attrib.normal);
	glDisableVertexAttribArray(attrib.uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawTriangles3D_AO(const Attrib &attrib, GLuint buffer, int count) {
	drawTriangles3D_AO_internal(attrib, buffer, count);
}

void drawTriangles3D_Text(const Attrib &attrib, GLuint buffer, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib.position);
	glEnableVertexAttribArray(attrib.uv);
	glVertexAttribPointer(attrib.position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, reinterpret_cast<GLvoid*>(0));
	glVertexAttribPointer(attrib.uv, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*3));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib.position);
	glDisableVertexAttribArray(attrib.uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawTriangles3D(const Attrib &attrib, GLuint buffer, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib.position);
	glEnableVertexAttribArray(attrib.normal);
	glEnableVertexAttribArray(attrib.uv);
	glVertexAttribPointer(attrib.position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, reinterpret_cast<GLvoid*>(0));
	glVertexAttribPointer(attrib.normal, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*3));
	glVertexAttribPointer(attrib.uv, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*6));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib.position);
	glDisableVertexAttribArray(attrib.normal);
	glDisableVertexAttribArray(attrib.uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawTriangles2D(const Attrib &attrib, GLuint buffer, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib.position);
	glEnableVertexAttribArray(attrib.uv);
	glVertexAttribPointer(attrib.position, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, reinterpret_cast<GLvoid*>(0));
	glVertexAttribPointer(attrib.uv, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*2));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib.position);
	glDisableVertexAttribArray(attrib.uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawLines(const Attrib &attrib, GLuint buffer, int components, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib.position);
	glVertexAttribPointer(attrib.position, components, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));
	glDrawArrays(GL_LINES, 0, count);
	glDisableVertexAttribArray(attrib.position);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// --- Fonctions de rendu d'objets ---

// Helper : configure la matrice de vue 3D et envoie les uniforms au shader.
static void setup3D(const Attrib &attrib, const State &cameraState, int width, int height, float fov, int ortho, int renderRadius) {
	float matrix[16];
	Utils::set_matrix_3d(matrix, width, height, cameraState.x, cameraState.y, cameraState.z,
						   cameraState.rx, cameraState.ry, fov, ortho, renderRadius);
	glUseProgram(attrib.program);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	glUniform3f(attrib.camera, cameraState.x, cameraState.y, cameraState.z);
	// Le timer (pour le jour/nuit) pourra être envoyé via un autre appel.
}

// Rendu du ciel.
void renderSky(const Attrib &attrib, const State &cameraState, GLuint skyBuffer,
			   int width, int height, int renderRadius, float fov, int ortho) {
	float matrix[16];
	// Pour le ciel, on place la caméra à l'origine.
	Utils::set_matrix_3d(matrix, width, height, 0, 0, 0, cameraState.rx, cameraState.ry, fov, 0, renderRadius);
	glUseProgram(attrib.program);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib.sampler, 2); // Texture du ciel.
	// Le timer (ex. time_of_day) pourra être défini ici.
	glUniform1f(attrib.timer, 0.0f);
	// On dessine un nombre fixe de triangles pour le ciel.
	drawTriangles3D(attrib, skyBuffer, 512 * 3);
}

// Rendu des chunks.
// Retourne le nombre total de faces affichées.
int renderChunks(const Attrib &attrib, const State &cameraState,
				 Chunk *chunks, int chunkCount, int renderRadius,
				 int width, int height, float fov, int ortho) {
	int faceCount = 0;
	setup3D(attrib, cameraState, width, height, fov, ortho, renderRadius);
	// Pour simplifier, nous ne refaisons pas ici le calcul de la frustum.
	for (int i = 0; i < chunkCount; i++) {
		Chunk *chunk = &chunks[i];
		// On dessine le chunk avec son buffer pré-généré.
		drawTriangles3D_AO(attrib, chunk->buffer, chunk->faces * FACE_VERTEX_COUNT);
		faceCount += chunk->faces;
	}
	return faceCount;
}

// Rendu des joueurs.
void renderPlayers(const Attrib &attrib, const State &cameraState,
				   Player *players, int playerCount,
				   int width, int height, float fov, int ortho) {
	setup3D(attrib, cameraState, width, height, fov, ortho, 0);
	for (int i = 0; i < playerCount; i++) {
		Player *player = &players[i];
		glBindBuffer(GL_ARRAY_BUFFER, player->buffer);
		glEnableVertexAttribArray(attrib.position);
		glEnableVertexAttribArray(attrib.normal);
		glEnableVertexAttribArray(attrib.uv);
		glVertexAttribPointer(attrib.position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*10, reinterpret_cast<GLvoid*>(0));
		glVertexAttribPointer(attrib.normal, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*10, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*3));
		glVertexAttribPointer(attrib.uv, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*10, reinterpret_cast<GLvoid*>(sizeof(GLfloat)*6));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDisableVertexAttribArray(attrib.position);
		glDisableVertexAttribArray(attrib.normal);
		glDisableVertexAttribArray(attrib.uv);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

// Rendu du crosshair.
void renderCrosshairs(const Attrib &attrib, int width, int height, int scale) {
	float matrix[16];
	Utils::set_matrix_2d(matrix, width, height);
	glUseProgram(attrib.program);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	GLuint buffer = genCrosshairBuffer(width, height, scale);
	drawLines(attrib, buffer, 2, 4);
	Utils::del_buffer(buffer);
}

// Rendu de l'item sélectionné.
void renderItem(const Attrib &attrib, int width, int height, int scale, int itemIndex) {
	float matrix[16];
	Utils::set_matrix_item(matrix, width, height, scale);
	glUseProgram(attrib.program);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	glUniform3f(attrib.camera, 0, 0, 5);
	glUniform1i(attrib.sampler, 0);
	glUniform1f(attrib.timer, 0.0f);
	// En fonction de l'item, on génère un cube ou une plante.
	if (itemIndex < 24) { // Par exemple, items < 24 sont traités comme plantes.
		GLuint buffer = genPlantBuffer(0, 0, 0, 0.5f, itemIndex);
		drawTriangles3D_AO(attrib, buffer, 24);
		Utils::del_buffer(buffer);
	} else {
		GLuint buffer = genCubeBuffer(0, 0, 0, 0.5f, itemIndex);
		drawTriangles3D_AO(attrib, buffer, 36);
		Utils::del_buffer(buffer);
	}
}

// Rendu du texte.
void renderText(const Attrib &attrib, int justify, float x, float y, float n,
				const char *text, int width, int height) {
	float matrix[16];
	Utils::set_matrix_2d(matrix, width, height);
	glUseProgram(attrib.program);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib.sampler, 1);
	glUniform1i(attrib.extra1, 0);
	int length = static_cast<int>(strlen(text));
	// Ajuster x selon la justification (0: left, 1: center, 2: right)
	x -= n * justify * (length - 1) / 2;
	GLuint buffer = genTextBuffer(x, y, n, text);
	drawTriangles2D(attrib, buffer, length * FACE_VERTEX_COUNT);
	Utils::del_buffer(buffer);
}

// Rendu des panneaux (signs).
void renderSigns(const Attrib &attrib, const State &cameraState,
				 Chunk *chunks, int chunkCount, int signRadius,
				 int width, int height, float fov, int ortho) {
	float matrix[16];
	Utils::set_matrix_3d(matrix, width, height, cameraState.x, cameraState.y, cameraState.z,
						   cameraState.rx, cameraState.ry, fov, ortho, signRadius * 16);
	float planes[6][4];
	Utils::frustum_planes(planes, signRadius, matrix);
	glUseProgram(attrib.program);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib.sampler, 3);
	glUniform1i(attrib.extra1, 1);
	// On parcourt les chunks et on dessine les panneaux de chacun.
	for (int i = 0; i < chunkCount; i++) {
		Chunk *chunk = &chunks[i];
		drawTriangles3D_Text(attrib, chunk->sign_buffer, chunk->sign_faces * FACE_VERTEX_COUNT);
	}
}

// Rendu du wireframe autour d’un bloc (exemple simplifié).
void renderWireframe(const Attrib &attrib, const State &cameraState,
					 Chunk *chunks, int chunkCount, int renderRadius,
					 int width, int height, float fov, int ortho) {
	float matrix[16];
	Utils::set_matrix_3d(matrix, width, height, cameraState.x, cameraState.y, cameraState.z,
						   cameraState.rx, cameraState.ry, fov, ortho, renderRadius * 16);
	// Ici, pour simplifier, nous générons un wireframe pour un bloc fictif aux coordonnées (0,0,0).
	GLfloat wireData[72];
	Cube::makeCubeWireframe(wireData, 0, 0, 0, 0.53f);
	GLuint buffer = Utils::gen_buffer(sizeof(wireData), wireData);
	
	glUseProgram(attrib.program);
	glLineWidth(1);
	glEnable(GL_COLOR_LOGIC_OP);
	glUniformMatrix4fv(attrib.matrix, 1, GL_FALSE, matrix);
	drawLines(attrib, buffer, 3, 24);
	Utils::del_buffer(buffer);
	glDisable(GL_COLOR_LOGIC_OP);
}

} // namespace RenderManager
