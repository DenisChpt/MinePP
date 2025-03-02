// main.cpp – Partie 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <curl/curl.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Auth.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Cube.hpp"
#include "Database.hpp"
#include "Item.hpp"
#include "Map.hpp"
#include "Matrix.hpp"
#include "Noise.hpp"
#include "Sign.hpp"
#include "Utils.hpp"
#include "World.hpp"

#include <iostream>

using std::max;
using std::min;

// Constantes globales
constexpr int MAX_CHUNKS = 1;
constexpr int MAX_PLAYERS = 128;
constexpr int WORKERS = 4;
constexpr int MAX_TEXT_LENGTH = 256;
constexpr int MAX_NAME_LENGTH = 32;
constexpr int MAX_PATH_LENGTH = 256;
constexpr int MAX_ADDR_LENGTH = 256;

constexpr int ALIGN_LEFT = 0;
constexpr int ALIGN_CENTER = 1;
constexpr int ALIGN_RIGHT = 2;

constexpr int MODE_OFFLINE = 0;
constexpr int MODE_ONLINE = 1;

constexpr int WORKER_IDLE = 0;
constexpr int WORKER_BUSY = 1;
constexpr int WORKER_DONE = 2;

#define CHUNK_SIZE (g->config->getChunkSize())
#define SCROLL_THRESHOLD (g->config->getScrollThreshold())
#define DEFAULT_PORT 4080

// Structures

struct Chunk
{
	World::Map map;
	World::Map lights;
	Utils::SignList signs;
	int p, q;
	int faces;
	int sign_faces;
	int dirty;
	int miny, maxy;
	GLuint buffer;
	GLuint sign_buffer;
	Chunk() : map(0, 0, 0, 0), lights(0, 0, 0, 0), p(0), q(0), faces(0), sign_faces(0), dirty(0), miny(0), maxy(0), buffer(0), sign_buffer(0) {}
};

struct WorkerItem
{
	int p, q;
	int load;
	World::Map *block_maps[3][3];
	World::Map *light_maps[3][3];
	int miny, maxy;
	int faces;
	GLfloat *data;
};

struct Worker
{
	int index;
	int state;
	std::thread thrd;
	std::mutex mtx;
	std::condition_variable cnd;
	WorkerItem item;
	Worker() : index(0), state(WORKER_IDLE) {}
};

struct Block
{
	int x, y, z, w;
};

struct State
{
	float x, y, z;
	float rx, ry;
	float t;
};

struct Player
{
	int id;
	char name[MAX_NAME_LENGTH];
	State state, state1, state2;
	GLuint buffer;
};

struct Attrib
{
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

struct Model
{
	GLFWwindow *window;
	Worker workers[WORKERS];
	Chunk chunks[MAX_CHUNKS];
	int chunk_count;
	int create_radius;
	int render_radius;
	int delete_radius;
	int sign_radius;
	Player players[MAX_PLAYERS];
	int player_count;
	int typing;
	char typing_buffer[MAX_TEXT_LENGTH];
	int message_index;
	char messages[4][MAX_TEXT_LENGTH]; // MAX_MESSAGES supposé être 4
	int width, height;
	int observe1, observe2;
	int flying;
	int item_index;
	int scale;
	int ortho;
	float fov;
	int suppress_char;
	int mode;
	int mode_changed;
	char db_path[MAX_PATH_LENGTH];
	char server_addr[MAX_ADDR_LENGTH];
	int server_port;
	int day_length;
	int time_changed;
	Block block0, block1, copy0, copy1;
	// Ajout d'un pointeur vers la configuration
	IConfig *config;
	Model() : chunk_count(0), player_count(0), typing(0), message_index(0),
			  width(0), height(0), observe1(0), observe2(0), flying(0), item_index(0),
			  scale(1), ortho(0), fov(65), suppress_char(0), mode(MODE_OFFLINE),
			  mode_changed(0), day_length(600), time_changed(0) { config = new Config(); }
};

static Model model;
static Model *g = &model;
static Database::DB *db = new Database::DB();
static Network::Client *client = new Network::Client();

// Fonctions utilitaires

// Renvoie le numéro de chunk (en divisant par CHUNK_SIZE)
int chunked(float x)
{
	return static_cast<int>(std::floor(std::round(x) / CHUNK_SIZE));
}

// Renvoie le temps du jour (cycle entre 0 et 1) basé sur glfwGetTime()
float time_of_day()
{
	if (g->day_length <= 0)
	{
		return 0.5f;
	}
	float t = static_cast<float>(glfwGetTime());
	t = t / g->day_length;
	t = t - static_cast<int>(t);
	return t;
}

// Calcule la luminosité du jour en fonction du temps
float get_daylight()
{
	float timer = time_of_day();
	if (timer < 0.5f)
	{
		float t = (timer - 0.25f) * 100;
		return 1.0f / (1.0f + std::pow(2, -t));
	}
	else
	{
		float t = (timer - 0.85f) * 100;
		return 1.0f - 1.0f / (1.0f + std::pow(2, -t));
	}
}

// Renvoie le facteur de mise à l'échelle basé sur la taille de la fenêtre
int get_scale_factor()
{
	int window_width, window_height;
	int buffer_width, buffer_height;
	glfwGetWindowSize(g->window, &window_width, &window_height);
	glfwGetFramebufferSize(g->window, &buffer_width, &buffer_height);
	int result = buffer_width / window_width;
	result = std::max(1, result);
	result = std::min(2, result);
	return result;
}

// Calcule le vecteur de regard (sight vector) à partir des angles de rotation
void get_sight_vector(float rx, float ry, float *vx, float *vy, float *vz)
{
	float m = std::cos(ry);
	*vx = std::cos(rx - Utils::radians(90)) * m;
	*vy = std::sin(ry);
	*vz = std::sin(rx - Utils::radians(90)) * m;
}

// Calcule le vecteur de déplacement (motion vector) en fonction des touches de mouvement
void get_motion_vector(int flying, int sz, int sx, float rx, float ry,
					   float *vx, float *vy, float *vz)
{
	*vx = *vy = *vz = 0;
	if (!sz && !sx)
		return;
	float strafe = std::atan2(static_cast<float>(sz), static_cast<float>(sx));
	if (flying)
	{
		float m = std::cos(ry);
		float yVal = std::sin(ry);
		if (sx)
		{
			if (!sz)
				yVal = 0;
			m = 1;
		}
		if (sz > 0)
			yVal = -yVal;
		*vx = std::cos(rx + strafe) * m;
		*vy = yVal;
		*vz = std::sin(rx + strafe) * m;
	}
	else
	{
		*vx = std::cos(rx + strafe);
		*vy = 0;
		*vz = std::sin(rx + strafe);
	}
}

// Génère un buffer pour le crosshair
GLuint gen_crosshair_buffer()
{
	int x = g->width / 2;
	int y = g->height / 2;
	int p = 10 * g->scale;
	float data[] = {
		static_cast<float>(x), static_cast<float>(y - p), static_cast<float>(x), static_cast<float>(y + p),
		static_cast<float>(x - p), static_cast<float>(y), static_cast<float>(x + p), static_cast<float>(y)};
	return Utils::gen_buffer(sizeof(data), data);
}

// --------------------------------------------------------------
// Fonctions de dessin – Rendu OpenGL
// --------------------------------------------------------------

static void drawTriangles3D_AO(const Attrib *attrib, GLuint buffer, int count)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib->position);
	glEnableVertexAttribArray(attrib->normal);
	glEnableVertexAttribArray(attrib->uv);
	glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 10, reinterpret_cast<GLvoid *>(0));
	glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 10, reinterpret_cast<GLvoid *>(sizeof(GLfloat) * 3));
	glVertexAttribPointer(attrib->uv, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 10, reinterpret_cast<GLvoid *>(sizeof(GLfloat) * 6));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib->position);
	glDisableVertexAttribArray(attrib->normal);
	glDisableVertexAttribArray(attrib->uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void drawTriangles3D_Text(const Attrib *attrib, GLuint buffer, int count)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib->position);
	glEnableVertexAttribArray(attrib->uv);
	glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, reinterpret_cast<GLvoid *>(0));
	glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, reinterpret_cast<GLvoid *>(sizeof(GLfloat) * 3));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib->position);
	glDisableVertexAttribArray(attrib->uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void drawTriangles3D(const Attrib *attrib, GLuint buffer, int count)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib->position);
	glEnableVertexAttribArray(attrib->normal);
	glEnableVertexAttribArray(attrib->uv);
	glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, reinterpret_cast<GLvoid *>(0));
	glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, reinterpret_cast<GLvoid *>(sizeof(GLfloat) * 3));
	glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, reinterpret_cast<GLvoid *>(sizeof(GLfloat) * 6));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib->position);
	glDisableVertexAttribArray(attrib->normal);
	glDisableVertexAttribArray(attrib->uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void drawTriangles2D(const Attrib *attrib, GLuint buffer, int count)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib->position);
	glEnableVertexAttribArray(attrib->uv);
	glVertexAttribPointer(attrib->position, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, reinterpret_cast<GLvoid *>(0));
	glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, reinterpret_cast<GLvoid *>(sizeof(GLfloat) * 2));
	glDrawArrays(GL_TRIANGLES, 0, count);
	glDisableVertexAttribArray(attrib->position);
	glDisableVertexAttribArray(attrib->uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void drawLines(const Attrib *attrib, GLuint buffer, int components, int count)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(attrib->position);
	glVertexAttribPointer(attrib->position, components, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid *>(0));
	glDrawArrays(GL_LINES, 0, count);
	glDisableVertexAttribArray(attrib->position);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void drawChunk(const Attrib *attrib, const Chunk *chunk)
{
	drawTriangles3D_AO(attrib, chunk->buffer, chunk->faces * 6);
}

static void drawItem(const Attrib *attrib, GLuint buffer, int count)
{
	drawTriangles3D_AO(attrib, buffer, count);
}

static void drawText(const Attrib *attrib, GLuint buffer, int length)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	drawTriangles2D(attrib, buffer, length * 6);
	glDisable(GL_BLEND);
	// Note : le buffer doit être supprimé après usage.
}

// --------------------------------------------------------------
// Fonctions de gestion des joueurs, chunks et collisions
// --------------------------------------------------------------

static Player *findPlayer(int id)
{
	for (int i = 0; i < g->player_count; i++)
	{
		Player *player = &g->players[i];
		if (player->id == id)
		{
			return player;
		}
	}
	return nullptr;
}

GLuint gen_player_buffer(float x, float y, float z, float rx, float ry)
{
	GLfloat *data = Utils::malloc_faces(10, 6);
	Cube::makeCharacter(data, x, y, z, rx, ry);
	return Utils::gen_faces(10, 6, data);
}

static void updatePlayer(Player *player, float x, float y, float z, float rx, float ry, int interpolate)
{
	if (interpolate)
	{
		State *s1 = &player->state1;
		State *s2 = &player->state2;
		std::memcpy(s1, s2, sizeof(State));
		s2->x = x;
		s2->y = y;
		s2->z = z;
		s2->rx = rx;
		s2->ry = ry;
		s2->t = static_cast<float>(glfwGetTime());
		if (s2->rx - s1->rx > PI)
		{
			s1->rx += 2 * PI;
		}
		if (s1->rx - s2->rx > PI)
		{
			s1->rx -= 2 * PI;
		}
	}
	else
	{
		State *s = &player->state;
		s->x = x;
		s->y = y;
		s->z = z;
		s->rx = rx;
		s->ry = ry;
		Utils::del_buffer(player->buffer);
		player->buffer = gen_player_buffer(s->x, s->y, s->z, s->rx, s->ry);
	}
}

static void interpolatePlayer(Player *player)
{
	State *s1 = &player->state1;
	State *s2 = &player->state2;
	float t1 = s2->t - s1->t;
	float t2 = static_cast<float>(glfwGetTime()) - s2->t;
	t1 = min(t1, 1.0f);
	t1 = max(t1, 0.1f);
	float p = min(t2 / t1, 1.0f);
	updatePlayer(player,
				 s1->x + (s2->x - s1->x) * p,
				 s1->y + (s2->y - s1->y) * p,
				 s1->z + (s2->z - s1->z) * p,
				 s1->rx + (s2->rx - s1->rx) * p,
				 s1->ry + (s2->ry - s1->ry) * p,
				 0);
}

static void deletePlayer(int id)
{
	Player *player = findPlayer(id);
	if (!player)
		return;
	int count = g->player_count;
	Utils::del_buffer(player->buffer);
	Player *other = &g->players[--count];
	std::memcpy(player, other, sizeof(Player));
	g->player_count = count;
}

static void deleteAllPlayers()
{
	for (int i = 0; i < g->player_count; i++)
	{
		Utils::del_buffer(g->players[i].buffer);
	}
	g->player_count = 0;
}

static float playerDistance(const Player *p1, const Player *p2)
{
	const State *s1 = &p1->state;
	const State *s2 = &p2->state;
	float dx = s2->x - s1->x;
	float dy = s2->y - s1->y;
	float dz = s2->z - s1->z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

static float playerCrosshairDistance(const Player *p1, const Player *p2)
{
	const State *s1 = &p1->state;
	const State *s2 = &p2->state;
	float d = playerDistance(p1, p2);
	float vx, vy, vz;
	get_sight_vector(s1->rx, s1->ry, &vx, &vy, &vz);
	vx *= d;
	vy *= d;
	vz *= d;
	float px = s1->x + vx, py = s1->y + vy, pz = s1->z + vz;
	float dx = s2->x - px, dy = s2->y - py, dz = s2->z - pz;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

static Player *playerCrosshair(Player *player)
{
	Player *result = nullptr;
	float threshold = Utils::radians(5);
	float best = 0;
	for (int i = 0; i < g->player_count; i++)
	{
		Player *other = &g->players[i];
		if (other == player)
			continue;
		float p = playerCrosshairDistance(player, other);
		float d = playerDistance(player, other);
		if (d < 96 && (p / d) < threshold)
		{
			if (best == 0 || d < best)
			{
				best = d;
				result = other;
			}
		}
	}
	return result;
}

// find_chunk, chunk_distance, chunk_visible, highest_block, _hit_test, hit_test,
// hit_test_face, collide, player_intersects_block
// Ces fonctions sont adaptées de main.c en utilisant std::abs, std::max, etc.

static Chunk *findChunk(int p, int q)
{
	for (int i = 0; i < g->chunk_count; i++)
	{
		Chunk *chunk = &g->chunks[i];
		if (chunk->p == p && chunk->q == q)
			return chunk;
	}
	return nullptr;
}

static int chunkDistance(const Chunk *chunk, int p, int q)
{
	int dp = abs(chunk->p - p);
	int dq = abs(chunk->q - q);
	return max(dp, dq);
}

static int chunkVisible(float planes[6][4], int p, int q, int miny, int maxy)
{
	int x = p * CHUNK_SIZE - 1;
	int z = q * CHUNK_SIZE - 1;
	int d = CHUNK_SIZE + 1;
	float points[8][3] = {
		{float(x + 0), float(miny), float(z + 0)},
		{float(x + d), float(miny), float(z + 0)},
		{float(x + 0), float(miny), float(z + d)},
		{float(x + d), float(miny), float(z + d)},
		{float(x + 0), float(maxy), float(z + 0)},
		{float(x + d), float(maxy), float(z + 0)},
		{float(x + 0), float(maxy), float(z + d)},
		{float(x + d), float(maxy), float(z + d)}};
	int n = g->ortho ? 4 : 6;
	for (int i = 0; i < n; i++)
	{
		int in = 0, out = 0;
		for (int j = 0; j < 8; j++)
		{
			float dPlane = planes[i][0] * points[j][0] +
						   planes[i][1] * points[j][1] +
						   planes[i][2] * points[j][2] +
						   planes[i][3];
			if (dPlane < 0)
				out++;
			else
				in++;
			if (in && out)
				break;
		}
		if (in == 0)
			return 0;
	}
	return 1;
}

static int highestBlock(float x, float z)
{
	int result = -1;
	int nx = roundf(x);
	int nz = roundf(z);
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		World::Map *map = &chunk->map;
		// Utilisation de la macro MAP_FOR_EACH adaptée ici par une boucle sur map->data
		for (unsigned int i = 0; i < map->getData().size(); i++)
		{
			const World::MapEntry entry = map->getData()[i];
			if (entry.value == 0)
				continue;
			if (entry.e.x + map->getDx() == nx && entry.e.z + map->getDz() == nz)
			{
				result = max(result, static_cast<int>(entry.e.y + map->getDy()));
			}
		}
	}
	return result;
}

// _hit_test et hit_test pour déterminer le bloc touché par un rayon de vue
static int _hitTest(const World::Map *map, float max_distance, int previous,
					float x, float y, float z,
					float vx, float vy, float vz,
					int *hx, int *hy, int *hz)
{
	int m = 32;
	int px = 0, py = 0, pz = 0;
	for (int i = 0; i < max_distance * m; i++)
	{
		int nx = roundf(x);
		int ny = roundf(y);
		int nz = roundf(z);
		if (nx != px || ny != py || nz != pz)
		{
			int hw = map->get(nx, ny, nz);
			if (hw > 0)
			{
				if (previous)
				{
					*hx = px;
					*hy = py;
					*hz = pz;
				}
				else
				{
					*hx = nx;
					*hy = ny;
					*hz = nz;
				}
				return hw;
			}
			px = nx;
			py = ny;
			pz = nz;
		}
		x += vx / m;
		y += vy / m;
		z += vz / m;
	}
	return 0;
}

static int hitTest(int previous, float x, float y, float z, float rx, float ry,
				   int *bx, int *by, int *bz)
{
	int result = 0;
	float best = 0;
	int p = chunked(x);
	int q = chunked(z);
	float vx, vy, vz;
	get_sight_vector(rx, ry, &vx, &vy, &vz);
	for (int i = 0; i < g->chunk_count; i++)
	{
		Chunk *chunk = &g->chunks[i];
		if (chunkDistance(chunk, p, q) > 1)
			continue;
		int hx, hy, hz;
		int hw = _hitTest(&chunk->map, 8, previous, x, y, z, vx, vy, vz, &hx, &hy, &hz);
		if (hw > 0)
		{
			float d = std::sqrt(powf(hx - x, 2) + powf(hy - y, 2) + powf(hz - z, 2));
			if (best == 0 || d < best)
			{
				best = d;
				*bx = hx;
				*by = hy;
				*bz = hz;
				result = hw;
			}
		}
	}
	return result;
}

static int hitTestFace(Player *player, int *x, int *y, int *z, int *face)
{
	State *s = &player->state;
	int w = hitTest(0, s->x, s->y, s->z, s->rx, s->ry, x, y, z);
	if (World::isObstacle(w))
	{
		int hx, hy, hz;
		hitTest(1, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
		int dx = hx - *x;
		int dy = hy - *y;
		int dz = hz - *z;
		if (dx == -1 && dy == 0 && dz == 0)
		{
			*face = 0;
			return 1;
		}
		if (dx == 1 && dy == 0 && dz == 0)
		{
			*face = 1;
			return 1;
		}
		if (dx == 0 && dy == 0 && dz == -1)
		{
			*face = 2;
			return 1;
		}
		if (dx == 0 && dy == 0 && dz == 1)
		{
			*face = 3;
			return 1;
		}
		if (dx == 0 && dy == 1 && dz == 0)
		{
			int degrees = roundf(Utils::degrees(atan2f(s->x - hx, s->z - hz)));
			if (degrees < 0)
			{
				degrees += 360;
			}
			int top = ((degrees + 45) / 90) % 4;
			*face = 4 + top;
			return 1;
		}
	}
	return 0;
}

static int collide(int height, float *x, float *y, float *z)
{
	int result = 0;
	int p = chunked(*x);
	int q = chunked(*z);
	Chunk *chunk = findChunk(p, q);
	if (!chunk)
		return result;
	World::Map *map = &chunk->map;
	int nx = roundf(*x);
	int ny = roundf(*y);
	int nz = roundf(*z);
	float px = *x - nx;
	float py = *y - ny;
	float pz = *z - nz;
	float pad = 0.25f;
	for (int dy = 0; dy < height; dy++)
	{
		if (px < -pad && World::isObstacle(map->get(nx - 1, ny - dy, nz)))
		{
			*x = nx - pad;
		}
		if (px > pad && World::isObstacle(map->get(nx + 1, ny - dy, nz)))
		{
			*x = nx + pad;
		}
		if (py < -pad && World::isObstacle(map->get(nx, ny - dy - 1, nz)))
		{
			*y = ny - pad;
			result = 1;
		}
		if (py > pad && World::isObstacle(map->get(nx, ny - dy + 1, nz)))
		{
			*y = ny + pad;
			result = 1;
		}
		if (pz < -pad && World::isObstacle(map->get(nx, ny - dy, nz - 1)))
		{
			*z = nz - pad;
		}
		if (pz > pad && World::isObstacle(map->get(nx, ny - dy, nz + 1)))
		{
			*z = nz + pad;
		}
	}
	return result;
}

static int playerIntersectsBlock(int height, float x, float y, float z, int hx, int hy, int hz)
{
	int nx = roundf(x), ny = roundf(y), nz = roundf(z);
	for (int i = 0; i < height; i++)
	{
		if (nx == hx && ny - i == hy && nz == hz)
			return 1;
	}
	return 0;
}

// _gen_sign_buffer et gen_sign_buffer pour générer la géométrie des panneaux (signs)
static int _genSignBuffer(GLfloat *data, float x, float y, float z, int face, const char *text)
{
	// Tableaux internes définissant les décalages pour la géométrie des glyphes
	static const int glyph_dx[8] = {0, 0, -1, 1, 1, 0, -1, 0};
	static const int glyph_dz[8] = {1, -1, 0, 0, 0, -1, 0, 1};
	static const int line_dx[8] = {0, 0, 0, 0, 0, 1, 0, -1};
	static const int line_dy[8] = {-1, -1, -1, -1, 0, 0, 0, 0};
	static const int line_dz[8] = {0, 0, 0, 0, 1, 0, -1, 0};

	if (face < 0 || face >= 8)
		return 0;
	int count = 0;
	float max_width = 64;
	float line_height = 1.25f;
	char lines[1024];
	// Utilise la fonction tokenize de Util pour découper le texte en lignes
	int rows = Utils::wrap(text, max_width, lines, 1024);
	rows = min(rows, 5);
	int dx = glyph_dx[face];
	int dz = glyph_dz[face];
	int ldx = line_dx[face];
	int ldy = line_dy[face];
	int ldz = line_dz[face];
	float n = 1.0f / (max_width / 10);
	float sx = x - n * (rows - 1) * (line_height / 2) * ldx;
	float sy = y - n * (rows - 1) * (line_height / 2) * ldy;
	float sz = z - n * (rows - 1) * (line_height / 2) * ldz;
	char *key;
	char *linePtr = Utils::tokenize(lines, "\n", &key);
	while (linePtr)
	{
		int length = strlen(linePtr);
		int line_width = Utils::string_width(linePtr);
		line_width = min(line_width, static_cast<int>(max_width));
		float rx = sx - dx * line_width / max_width / 2;
		float ry = sy;
		float rz = sz - dz * line_width / max_width / 2;
		for (int i = 0; i < length; i++)
		{
			int width = Utils::char_width(linePtr[i]);
			line_width -= width;
			if (line_width < 0)
				break;
			rx += dx * width / max_width / 2;
			rz += dz * width / max_width / 2;
			if (linePtr[i] != ' ')
			{
				Cube::makeCharacter3D(data + count * 30, rx, ry, rz, n / 2, face, linePtr[i]);
				count++;
			}
			rx += dx * width / max_width / 2;
			rz += dz * width / max_width / 2;
		}
		sx += n * line_height * ldx;
		sy += n * line_height * ldy;
		sz += n * line_height * ldz;
		linePtr = Utils::tokenize(nullptr, "\n", &key);
		rows--;
		if (rows <= 0)
			break;
	}
	return count;
}

static GLuint genSignBuffer(Chunk *chunk)
{
	Utils::SignList *signs = &chunk->signs;
	// Première passe : compter le nombre de glyphes
	int max_faces = 0;
	for (size_t i = 0; i < signs->size(); i++)
	{
		max_faces += (int)strlen(signs->getSigns()[i].text.c_str());
	}
	// Deuxième passe : générer la géométrie
	GLfloat *data = Utils::malloc_faces(5, max_faces);
	int faces = 0;
	for (size_t i = 0; i < signs->size(); i++)
	{
		const Utils::Sign *e = &signs->getSigns()[i];
		faces += _genSignBuffer(data + faces * 30, (float)e->x, (float)e->y, (float)e->z, e->face, e->text.c_str());
	}
	Utils::del_buffer(chunk->sign_buffer);
	chunk->sign_buffer = Utils::gen_faces(5, faces, data);
	chunk->sign_faces = faces;
	return chunk->sign_buffer;
}

// --------------------------------------------------------------
// Fonctions de génération et gestion des chunks et workers
// --------------------------------------------------------------

#define XZ_SIZE (CHUNK_SIZE * 3 + 2)
#define XZ_LO (CHUNK_SIZE)
#define XZ_HI (CHUNK_SIZE * 2 + 1)
#define Y_SIZE 258
#define XYZ(x, y, z) ((y) * XZ_SIZE * XZ_SIZE + (x) * XZ_SIZE + (z))
#define XZ(x, z) ((x) * XZ_SIZE + (z))

void light_fill(
	char *opaque, char *light,
	int x, int y, int z, int w, int force)
{
	if (x + w < XZ_LO || z + w < XZ_LO)
	{
		return;
	}
	if (x - w > XZ_HI || z - w > XZ_HI)
	{
		return;
	}
	if (y < 0 || y >= Y_SIZE)
	{
		return;
	}
	if (light[XYZ(x, y, z)] >= w)
	{
		return;
	}
	if (!force && opaque[XYZ(x, y, z)])
	{
		return;
	}
	light[XYZ(x, y, z)] = w--;
	light_fill(opaque, light, x - 1, y, z, w, 0);
	light_fill(opaque, light, x + 1, y, z, w, 0);
	light_fill(opaque, light, x, y - 1, z, w, 0);
	light_fill(opaque, light, x, y + 1, z, w, 0);
	light_fill(opaque, light, x, y, z - 1, w, 0);
	light_fill(opaque, light, x, y, z + 1, w, 0);
}

void occlusion(
	char neighbors[27], char lights[27], float shades[27],
	float ao[6][4], float light[6][4])
{
	static const int lookup3[6][4][3] = {
		{{0, 1, 3}, {2, 1, 5}, {6, 3, 7}, {8, 5, 7}},
		{{18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25}},
		{{6, 7, 15}, {8, 7, 17}, {24, 15, 25}, {26, 17, 25}},
		{{0, 1, 9}, {2, 1, 11}, {18, 9, 19}, {20, 11, 19}},
		{{0, 3, 9}, {6, 3, 15}, {18, 9, 21}, {24, 15, 21}},
		{{2, 5, 11}, {8, 5, 17}, {20, 11, 23}, {26, 17, 23}}};
	static const int lookup4[6][4][4] = {
		{{0, 1, 3, 4}, {1, 2, 4, 5}, {3, 4, 6, 7}, {4, 5, 7, 8}},
		{{18, 19, 21, 22}, {19, 20, 22, 23}, {21, 22, 24, 25}, {22, 23, 25, 26}},
		{{6, 7, 15, 16}, {7, 8, 16, 17}, {15, 16, 24, 25}, {16, 17, 25, 26}},
		{{0, 1, 9, 10}, {1, 2, 10, 11}, {9, 10, 18, 19}, {10, 11, 19, 20}},
		{{0, 3, 9, 12}, {3, 6, 12, 15}, {9, 12, 18, 21}, {12, 15, 21, 24}},
		{{2, 5, 11, 14}, {5, 8, 14, 17}, {11, 14, 20, 23}, {14, 17, 23, 26}}};
	static const float curve[4] = {0.0, 0.25, 0.5, 0.75};
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int corner = neighbors[lookup3[i][j][0]];
			int side1 = neighbors[lookup3[i][j][1]];
			int side2 = neighbors[lookup3[i][j][2]];
			int value = side1 && side2 ? 3 : corner + side1 + side2;
			float shade_sum = 0;
			float light_sum = 0;
			int is_light = lights[13] == 15;
			for (int k = 0; k < 4; k++)
			{
				shade_sum += shades[lookup4[i][j][k]];
				light_sum += lights[lookup4[i][j][k]];
			}
			if (is_light)
			{
				light_sum = 15 * 4 * 10;
			}
			float total = curve[value] + shade_sum / 4.0;
			ao[i][j] = min((double)total, 1.0);
			light[i][j] = light_sum / 15.0 / 4.0;
		}
	}
}

static void computeChunk(WorkerItem *item)
{
	// Allocation d'arrays pour opaque, lumière et hauteur
	char *opaque = (char *)calloc(XZ_SIZE * XZ_SIZE * Y_SIZE, sizeof(char));
	char *light = (char *)calloc(XZ_SIZE * XZ_SIZE * Y_SIZE, sizeof(char));
	char *highest = (char *)calloc(XZ_SIZE * XZ_SIZE, sizeof(char));

	int ox = item->p * CHUNK_SIZE - CHUNK_SIZE - 1;
	int oy = -1;
	int oz = item->q * CHUNK_SIZE - CHUNK_SIZE - 1;

	// Vérifie la présence de lumière
	int has_light = 0;
	if (g->config->showLights())
	{
		for (int a = 0; a < 3; a++)
		{
			for (int b = 0; b < 3; b++)
			{
				World::Map *map = item->light_maps[a][b];
				if (map && map->getSize() > 0)
					has_light = 1;
			}
		}
	}

	// Remplit le tableau opaque
	for (int a = 0; a < 3; a++)
	{
		for (int b = 0; b < 3; b++)
		{
			World::Map *map = item->block_maps[a][b];
			if (!map)
				continue;
			// Boucle sur toutes les entrées du map
			unsigned int size = map->getData().size();
			for (unsigned int i = 0; i < map->getData().size(); i++)
			{
				std::cout << i << "/" << size << std::endl;
				const World::MapEntry entry = map->getData()[i];
				if (entry.value == 0)
					continue;
				std::cout << "entry: " << entry.e.x << " " << entry.e.y << " " << entry.e.z << " " << entry.e.w << std::endl;
				int x = entry.e.x + map->getDx();
				int y = entry.e.y + map->getDy();
				int z = entry.e.z + map->getDz();
				if (x < 0 || y < 0 || z < 0 || x >= XZ_SIZE || y >= Y_SIZE || z >= XZ_SIZE)
					continue;
				std::cout << "test1" << std::endl;
				opaque[XYZ(x, y, z)] = !World::isTransparent(entry.e.w);
				std::cout << "test2" << std::endl;
				if (opaque[XYZ(x, y, z)])
				{
					highest[XZ(x, z)] = max(highest[XZ(x, z)], (char)y);
				}
				std::cout << "a" << a << " b" << b << std::endl;
			}
		}
	}

	// Remplit la lumière par flood fill si besoin
	if (has_light)
	{
		for (int a = 0; a < 3; a++)
		{
			for (int b = 0; b < 3; b++)
			{
				World::Map *map = item->light_maps[a][b];
				if (!map)
					continue;
				for (unsigned int i = 0; i < map->getData().size(); i++)
				{
					const World::MapEntry entry = map->getData()[i];
					int x = entry.e.x + map->getDx();
					int y = entry.e.y + map->getDy();
					int z = entry.e.z + map->getDz();
					light_fill(opaque, light, x, y, z, entry.e.w, 1);
				}
			}
		}
	}

	World::Map *map = item->block_maps[1][1];
	int miny = 256, maxy = 0, faces = 0;
	// Parcours de la map centrale pour compter les faces exposées
	for (unsigned int i = 0; i < map->getData().size(); i++)
	{
		const World::MapEntry entry = map->getData()[i];
		if (entry.value == 0 || entry.e.w <= 0)
			continue;
		int x = entry.e.x + map->getDx();
		int y = entry.e.y + map->getDy();
		int z = entry.e.z + map->getDz();
		int f1 = !opaque[XYZ(x - 1, y, z)];
		int f2 = !opaque[XYZ(x + 1, y, z)];
		int f3 = !opaque[XYZ(x, y + 1, z)];
		int f4 = (!opaque[XYZ(x, y - 1, z)] && (entry.e.y > 0));
		int f5 = !opaque[XYZ(x, y, z - 1)];
		int f6 = !opaque[XYZ(x, y, z + 1)];
		int total = f1 + f2 + f3 + f4 + f5 + f6;
		if (total == 0)
			continue;
		if (World::isPlant(entry.e.w))
			total = 4;
		miny = min(miny, (int)entry.e.y);
		maxy = max(maxy, (int)entry.e.y);
		faces += total;
	}

	GLfloat *data = Utils::malloc_faces(10, faces);
	int offset = 0;
	for (unsigned int i = 0; i < map->getData().size(); i++)
	{
		const World::MapEntry entry = map->getData()[i];
		if (entry.value == 0 || entry.e.w <= 0)
			continue;
		int x = entry.e.x + map->getDx();
		int y = entry.e.y + map->getDy();
		int z = entry.e.z + map->getDz();
		int f1 = !opaque[XYZ(x - 1, y, z)];
		int f2 = !opaque[XYZ(x + 1, y, z)];
		int f3 = !opaque[XYZ(x, y + 1, z)];
		int f4 = (!opaque[XYZ(x, y - 1, z)] && (entry.e.y > 0));
		int f5 = !opaque[XYZ(x, y, z - 1)];
		int f6 = !opaque[XYZ(x, y, z + 1)];
		int total = f1 + f2 + f3 + f4 + f5 + f6;
		if (total == 0)
			continue;
		char neighbors[27] = {0}, lightsArr[27] = {0};
		float shades[27] = {0};
		int idx = 0;
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++)
			{
				for (int dz = -1; dz <= 1; dz++)
				{
					neighbors[idx] = opaque[XYZ(x + dx, y + dy, z + dz)];
					lightsArr[idx] = light[XYZ(x + dx, y + dy, z + dz)];
					shades[idx] = 0;
					if (y + dy <= highest[XZ(x + dx, z + dz)])
					{
						for (int oy = 0; oy < 8; oy++)
						{
							if (opaque[XYZ(x + dx, y + dy + oy, z + dz)])
							{
								shades[idx] = 1.0f - oy * 0.125f;
								break;
							}
						}
					}
					idx++;
				}
			}
		}
		float aoMat[6][4], lightMat[6][4];
		occlusion(neighbors, lightsArr, shades, aoMat, lightMat);
		if (World::isPlant(entry.e.w))
		{
			total = 4;
			float min_ao = 1, max_light = 0;
			for (int a = 0; a < 6; a++)
			{
				for (int b = 0; b < 4; b++)
				{
					min_ao = min(min_ao, aoMat[a][b]);
					max_light = max(max_light, lightMat[a][b]);
				}
			}
			float rotation = Noise::simplex2(x, z, 4, 0.5, 2) * 360;
			Cube::makePlant(data + offset, min_ao, max_light, x, y, z, 0.5f, entry.e.w, rotation);
		}
		else
		{
			Cube::makeCube(data + offset, aoMat, lightMat, f1, f2, f3, f4, f5, f6, x, y, z, 0.5f, entry.e.w);
		}
		offset += total * 60;
	}
	free(opaque);
	free(light);
	free(highest);
	item->miny = miny;
	item->maxy = maxy;
	item->faces = faces;
	item->data = data;
}

static void generateChunk(Chunk *chunk, WorkerItem *item)
{
	chunk->miny = item->miny;
	chunk->maxy = item->maxy;
	chunk->faces = item->faces;
	Utils::del_buffer(chunk->buffer);
	chunk->buffer = Utils::gen_faces(10, item->faces, item->data);
	genSignBuffer(chunk);
	chunk->dirty = 0;
}

void map_set_func(int x, int y, int z, int w, void *arg)
{
	World::Map *map = static_cast<World::Map *>(arg);
	map->set(x, y, z, w);
}

void request_chunk(int p, int q)
{
	int key = db->getKey(p, q);
	client->chunk(p, q, key);
}

static void genChunkBuffer(Chunk *chunk)
{
	WorkerItem item;
	item.p = chunk->p;
	item.q = chunk->q;
	for (int dp = -1; dp <= 1; dp++)
	{
		for (int dq = -1; dq <= 1; dq++)
		{
			Chunk *other = (dp || dq) ? findChunk(chunk->p + dp, chunk->q + dq) : chunk;
			if (other)
			{
				item.block_maps[dp + 1][dq + 1] = new World::Map(other->map);
				item.light_maps[dp + 1][dq + 1] = new World::Map(other->lights);
			}
			else
			{
				item.block_maps[dp + 1][dq + 1] = nullptr;
				item.light_maps[dp + 1][dq + 1] = nullptr;
			}
		}
	}
	// load_chunk et request_chunk (appelant create_world, db_load_*, etc.)
	World::createWorld(*g->config, chunk->p, chunk->q, map_set_func, &chunk->map);
	db->loadBlocks(chunk->map, chunk->p, chunk->q);
	db->loadLights(chunk->lights, chunk->p, chunk->q);
	request_chunk(chunk->p, chunk->q);
	computeChunk(&item);
	generateChunk(chunk, &item);
}

int has_lights(Chunk *chunk)
{
	if (!g->config->showLights())
	{
		return 0;
	}
	for (int dp = -1; dp <= 1; dp++)
	{
		for (int dq = -1; dq <= 1; dq++)
		{
			Chunk *other = chunk;
			if (dp || dq)
			{
				other = findChunk(chunk->p + dp, chunk->q + dq);
			}
			if (!other)
			{
				continue;
			}
			World::Map *map = &other->lights;
			if (map->getSize())
			{
				return 1;
			}
		}
	}
	return 0;
}

// Chunk *find_chunk(int p, int q)
// {
// 	for (int i = 0; i < g->chunk_count; i++)
// 	{
// 		Chunk *chunk = g->chunks + i;
// 		if (chunk->p == p && chunk->q == q)
// 		{
// 			return chunk;
// 		}
// 	}
// 	return 0;
// }

void dirty_chunk(Chunk *chunk)
{
	chunk->dirty = 1;
	if (has_lights(chunk))
	{
		for (int dp = -1; dp <= 1; dp++)
		{
			for (int dq = -1; dq <= 1; dq++)
			{
				Chunk *other = findChunk(chunk->p + dp, chunk->q + dq);
				if (other)
				{
					other->dirty = 1;
				}
			}
		}
	}
}

static void initChunk(Chunk *chunk, int p, int q)
{
	chunk->p = p;
	chunk->q = q;
	chunk->faces = 0;
	chunk->sign_faces = 0;
	chunk->buffer = 0;
	chunk->sign_buffer = 0;
	dirty_chunk(chunk);
	Utils::SignList signs;
	chunk->signs = signs; // Allocation initiale dans SignList
	db->loadSigns(chunk->signs, p, q);
	chunk->map = World::Map(p * CHUNK_SIZE - 1, 0, q * CHUNK_SIZE - 1, 0x7fff);
	chunk->lights = World::Map(p * CHUNK_SIZE - 1, 0, q * CHUNK_SIZE - 1, 0xf);
}

void load_chunk(WorkerItem *item)
{
	int p = item->p;
	int q = item->q;
	World::Map *block_map = item->block_maps[1][1];
	World::Map *light_map = item->light_maps[1][1];
	World::createWorld(*g->config, p, q, map_set_func, block_map);
	db->loadBlocks(*block_map, p, q);
	db->loadLights(*light_map, p, q);
}

static void createChunk(Chunk *chunk, int p, int q)
{
	initChunk(chunk, p, q);
	WorkerItem item;
	item.p = chunk->p;
	item.q = chunk->q;
	item.block_maps[1][1] = &chunk->map;
	item.light_maps[1][1] = &chunk->lights;
	load_chunk(&item);
	request_chunk(p, q);
}

// Fonctions worker et gestion des chunks (force_chunks, ensure_chunks_worker, etc.)
// (Le code est transcrit de main.c en adaptant la synchronisation via std::mutex et std::condition_variable.)
static void checkWorkers()
{
	for (int i = 0; i < WORKERS; i++)
	{
		Worker *worker = &g->workers[i];
		std::lock_guard<std::mutex> lock(worker->mtx);
		if (worker->state == WORKER_DONE)
		{
			WorkerItem *item = &worker->item;
			Chunk *chunk = findChunk(item->p, item->q);
			if (chunk)
			{
				if (item->load)
				{
					World::Map *block_map = item->block_maps[1][1];
					World::Map *light_map = item->light_maps[1][1];
					chunk->map.clear();
					chunk->lights.clear();
					chunk->map = *block_map;
					chunk->lights = *light_map;
					request_chunk(item->p, item->q);
				}
				generateChunk(chunk, item);
			}
			// Libération de la mémoire des maps copiées
			for (int a = 0; a < 3; a++)
			{
				for (int b = 0; b < 3; b++)
				{
					if (item->block_maps[a][b])
					{
						delete item->block_maps[a][b];
					}
					if (item->light_maps[a][b])
					{
						delete item->light_maps[a][b];
					}
				}
			}
			worker->state = WORKER_IDLE;
		}
	}
}

static void forceChunks(Player *player)
{
	State *s = &player->state;
	int p = chunked(s->x);
	int q = chunked(s->z);
	int r = 1;
	for (int dp = -r; dp <= r; dp++)
	{
		for (int dq = -r; dq <= r; dq++)
		{
			int a = p + dp;
			int b = q + dq;
			Chunk *chunk = findChunk(a, b);
			if (chunk)
			{
				if (chunk->dirty)
					genChunkBuffer(chunk);
			}
			else if (g->chunk_count < MAX_CHUNKS)
			{
				chunk = &g->chunks[g->chunk_count++];
				createChunk(chunk, a, b);
				genChunkBuffer(chunk);
			}
		}
	}
}

static void ensureChunksWorker(Player *player, Worker *worker)
{
	// Détermine la zone de création en fonction de la vue
	State *s = &player->state;
	float matrix[16];
	Utils::set_matrix_3d(matrix, g->width, g->height, s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
	float planes[6][4];
	Utils::frustum_planes(planes, g->render_radius, matrix);
	int p = chunked(s->x), q = chunked(s->z);
	int r = g->create_radius;
	int best_score = 0x0fffffff, best_a = 0, best_b = 0;
	for (int dp = -r; dp <= r; dp++)
	{
		for (int dq = -r; dq <= r; dq++)
		{
			int a = p + dp, b = q + dq;
			int index = (abs(a) ^ abs(b)) % WORKERS;
			if (index != worker->index)
				continue;
			Chunk *chunk = findChunk(a, b);
			if (chunk && !chunk->dirty)
				continue;
			int distance = max(abs(dp), abs(dq));
			int invisible = !chunkVisible(planes, a, b, 0, 256);
			int priority = (chunk && chunk->buffer && chunk->dirty) ? 1 : 0;
			int score = (invisible << 24) | (priority << 16) | distance;
			if (score < best_score)
			{
				best_score = score;
				best_a = a;
				best_b = b;
			}
		}
	}
	if (best_score == 0x0fffffff)
		return;
	int a = best_a, b = best_b;
	int load = 0;
	Chunk *chunk = findChunk(a, b);
	if (!chunk)
	{
		load = 1;
		if (g->chunk_count < MAX_CHUNKS)
		{
			chunk = &g->chunks[g->chunk_count++];
			initChunk(chunk, a, b);
		}
		else
			return;
	}
	WorkerItem *item = &worker->item;
	item->p = chunk->p;
	item->q = chunk->q;
	item->load = load;
	for (int dp = -1; dp <= 1; dp++)
	{
		for (int dq = -1; dq <= 1; dq++)
		{
			Chunk *other = (dp || dq) ? findChunk(chunk->p + dp, chunk->q + dq) : chunk;
			if (other)
			{
				item->block_maps[dp + 1][dq + 1] = new World::Map(other->map);
				item->light_maps[dp + 1][dq + 1] = new World::Map(other->lights);
			}
			else
			{
				item->block_maps[dp + 1][dq + 1] = nullptr;
				item->light_maps[dp + 1][dq + 1] = nullptr;
			}
		}
	}
	chunk->dirty = 0;
	worker->state = WORKER_BUSY;
	worker->cnd.notify_one();
}

static void ensureChunks(Player *player)
{
	checkWorkers();
	forceChunks(player);
	for (int i = 0; i < WORKERS; i++)
	{
		Worker *worker = &g->workers[i];
		std::lock_guard<std::mutex> lock(worker->mtx);
		if (worker->state == WORKER_IDLE)
		{
			ensureChunksWorker(player, worker);
		}
	}
}

// Fonction exécutée par le worker
static void workerRun(Worker *worker)
{
	bool running = true;
	while (running)
	{
		{
			std::unique_lock<std::mutex> lock(worker->mtx);
			while (worker->state != WORKER_BUSY)
				worker->cnd.wait(lock);
		}
		WorkerItem *item = &worker->item;
		if (item->load)
			load_chunk(item);
		computeChunk(item);
		{
			std::lock_guard<std::mutex> lock(worker->mtx);
			worker->state = WORKER_DONE;
		}
	}
}

// --------------------------------------------------------------
// Gestion des événements et saisie utilisateur
// --------------------------------------------------------------

void toggle_light(int x, int y, int z)
{
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		World::Map *map = &chunk->lights;
		int w = map->get(x, y, z) ? 0 : 15;
		map->set(x, y, z, w);
		db->insertLight(p, q, x, y, z, w);
		client->light(x, y, z, w);
		dirty_chunk(chunk);
	}
}

static void onLight()
{
	State *s = &g->players[0].state;
	int hx, hy, hz;
	int hw = hitTest(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
	if (hy > 0 && hy < 256 && World::isDestructable(hw))
		toggle_light(hx, hy, hz);
}

void unset_sign(int x, int y, int z)
{
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		Utils::SignList *signs = &chunk->signs;
		if (signs->remove_all(x, y, z))
		{
			chunk->dirty = 1;
			db->deleteSigns(x, y, z);
		}
	}
	else
	{
		db->deleteSigns(x, y, z);
	}
}

void set_light(int p, int q, int x, int y, int z, int w)
{
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		World::Map *map = &chunk->lights;
		if (map->set(x, y, z, w))
		{
			dirty_chunk(chunk);
			db->insertLight(p, q, x, y, z, w);
		}
	}
	else
	{
		db->insertLight(p, q, x, y, z, w);
	}
}

void _set_block(int p, int q, int x, int y, int z, int w, int dirty)
{
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		World::Map *map = &chunk->map;
		if (map->set(x, y, z, w))
		{
			if (dirty)
			{
				dirty_chunk(chunk);
			}
			db->insertBlock(p, q, x, y, z, w);
		}
	}
	else
	{
		db->insertBlock(p, q, x, y, z, w);
	}
	if (w == 0 && chunked(x) == p && chunked(z) == q)
	{
		unset_sign(x, y, z);
		set_light(p, q, x, y, z, 0);
	}
}

void set_block(int x, int y, int z, int w)
{
	int p = chunked(x);
	int q = chunked(z);
	_set_block(p, q, x, y, z, w, 1);
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dz = -1; dz <= 1; dz++)
		{
			if (dx == 0 && dz == 0)
			{
				continue;
			}
			if (dx && chunked(x + dx) == p)
			{
				continue;
			}
			if (dz && chunked(z + dz) == q)
			{
				continue;
			}
			_set_block(p + dx, q + dz, x, y, z, -w, 1);
		}
	}
	client->block(x, y, z, w);
}

void record_block(int x, int y, int z, int w)
{
	memcpy(&g->block1, &g->block0, sizeof(Block));
	g->block0.x = x;
	g->block0.y = y;
	g->block0.z = z;
	g->block0.w = w;
}

int get_block(int x, int y, int z)
{
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		World::Map *map = &chunk->map;
		return map->get(x, y, z);
	}
	return 0;
}

static void onLeftClick()
{
	State *s = &g->players[0].state;
	int hx, hy, hz;
	int hw = hitTest(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
	if (hy > 0 && hy < 256 && World::isDestructable(hw))
	{
		set_block(hx, hy, hz, 0);
		record_block(hx, hy, hz, 0);
		if (World::isPlant(get_block(hx, hy + 1, hz)))
			set_block(hx, hy + 1, hz, 0);
	}
}

static void onRightClick()
{
	State *s = &g->players[0].state;
	int hx, hy, hz;
	int hw = hitTest(1, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
	if (hy > 0 && hy < 256 && World::isObstacle(hw))
	{
		if (!playerIntersectsBlock(2, s->x, s->y, s->z, hx, hy, hz))
			set_block(hx, hy, hz, World::items[g->item_index]);
	}
}

static void onMiddleClick()
{
	State *s = &g->players[0].state;
	int hx, hy, hz;
	int hw = hitTest(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
	for (int i = 0; i < World::item_count; i++)
	{
		if (World::items[i] == hw)
		{
			g->item_index = i;
			break;
		}
	}
}

void unset_sign_face(int x, int y, int z, int face)
{
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		Utils::SignList *signs = &chunk->signs;
		if (signs->remove(x, y, z, face))
		{
			chunk->dirty = 1;
			db->deleteSign(x, y, z, face);
		}
	}
	else
	{
		db->deleteSign(x, y, z, face);
	}
}

void _set_sign(
	int p, int q, int x, int y, int z, int face, const char *text, int dirty)
{
	if (strlen(text) == 0)
	{
		unset_sign_face(x, y, z, face);
		return;
	}
	Chunk *chunk = findChunk(p, q);
	if (chunk)
	{
		Utils::SignList *signs = &chunk->signs;
		signs->add(x, y, z, face, text);
		if (dirty)
		{
			chunk->dirty = 1;
		}
	}
	db->insertSign(p, q, x, y, z, face, text);
}

void set_sign(int x, int y, int z, int face, const char *text)
{
	int p = chunked(x);
	int q = chunked(z);
	_set_sign(p, q, x, y, z, face, text, 1);
	client->sign(x, y, z, face, text);
}

void add_message(const char *text)
{
	printf("%s\n", text);
	snprintf(
		g->messages[g->message_index], MAX_TEXT_LENGTH, "%s", text);
	g->message_index = (g->message_index + 1) % g->config->getMaxMessages();
}

void login()
{
	std::string username;
	std::string identity_token;

	std::string access_token;
	if (db->authGetSelected(username, identity_token))
	{
		printf("Contacting login server for username: %s\n", username.c_str());
		if (Auth::getAccessToken(access_token, username, identity_token))
		{
			printf("Successfully authenticated with the login server\n");
			client->login(username, access_token);
		}
		else
		{
			printf("Failed to authenticate with the login server\n");
			client->login("", "");
		}
	}
	else
	{
		printf("Logging in anonymously\n");
		client->login("", "");
	}
}

void copy()
{
	memcpy(&g->copy0, &g->block0, sizeof(Block));
	memcpy(&g->copy1, &g->block1, sizeof(Block));
}

void builder_block(int x, int y, int z, int w)
{
	if (y <= 0 || y >= 256)
	{
		return;
	}
	if (World::isDestructable(get_block(x, y, z)))
	{
		set_block(x, y, z, 0);
	}
	if (w)
	{
		set_block(x, y, z, w);
	}
}

void paste()
{
	Block *c1 = &g->copy1;
	Block *c2 = &g->copy0;
	Block *p1 = &g->block1;
	Block *p2 = &g->block0;
	int scx = Utils::sign(c2->x - c1->x);
	int scz = Utils::sign(c2->z - c1->z);
	int spx = Utils::sign(p2->x - p1->x);
	int spz = Utils::sign(p2->z - p1->z);
	int oy = p1->y - c1->y;
	int dx = Utils::abs(c2->x - c1->x);
	int dz = Utils::abs(c2->z - c1->z);
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x <= dx; x++)
		{
			for (int z = 0; z <= dz; z++)
			{
				int w = get_block(c1->x + x * scx, y, c1->z + z * scz);
				builder_block(p1->x + x * spx, y + oy, p1->z + z * spz, w);
			}
		}
	}
}

void tree(Block *block)
{
	int bx = block->x;
	int by = block->y;
	int bz = block->z;
	for (int y = by + 3; y < by + 8; y++)
	{
		for (int dx = -3; dx <= 3; dx++)
		{
			for (int dz = -3; dz <= 3; dz++)
			{
				int dy = y - (by + 4);
				int d = (dx * dx) + (dy * dy) + (dz * dz);
				if (d < 11)
				{
					builder_block(bx + dx, y, bz + dz, 15);
				}
			}
		}
	}
	for (int y = by; y < by + 7; y++)
	{
		builder_block(bx, y, bz, 5);
	}
}

void array(Block *b1, Block *b2, int xc, int yc, int zc)
{
	if (b1->w != b2->w)
	{
		return;
	}
	int w = b1->w;
	int dx = b2->x - b1->x;
	int dy = b2->y - b1->y;
	int dz = b2->z - b1->z;
	xc = dx ? xc : 1;
	yc = dy ? yc : 1;
	zc = dz ? zc : 1;
	for (int i = 0; i < xc; i++)
	{
		int x = b1->x + dx * i;
		for (int j = 0; j < yc; j++)
		{
			int y = b1->y + dy * j;
			for (int k = 0; k < zc; k++)
			{
				int z = b1->z + dz * k;
				builder_block(x, y, z, w);
			}
		}
	}
}

void cube(Block *b1, Block *b2, int fill)
{
	if (b1->w != b2->w)
	{
		return;
	}
	int w = b1->w;
	int x1 = min(b1->x, b2->x);
	int y1 = min(b1->y, b2->y);
	int z1 = min(b1->z, b2->z);
	int x2 = max(b1->x, b2->x);
	int y2 = max(b1->y, b2->y);
	int z2 = max(b1->z, b2->z);
	int a = (x1 == x2) + (y1 == y2) + (z1 == z2);
	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			for (int z = z1; z <= z2; z++)
			{
				if (!fill)
				{
					int n = 0;
					n += x == x1 || x == x2;
					n += y == y1 || y == y2;
					n += z == z1 || z == z2;
					if (n <= a)
					{
						continue;
					}
				}
				builder_block(x, y, z, w);
			}
		}
	}
}

void sphere(Block *center, int radius, int fill, int fx, int fy, int fz)
{
	static const float offsets[8][3] = {
		{-0.5, -0.5, -0.5},
		{-0.5, -0.5, 0.5},
		{-0.5, 0.5, -0.5},
		{-0.5, 0.5, 0.5},
		{0.5, -0.5, -0.5},
		{0.5, -0.5, 0.5},
		{0.5, 0.5, -0.5},
		{0.5, 0.5, 0.5}};
	int cx = center->x;
	int cy = center->y;
	int cz = center->z;
	int w = center->w;
	for (int x = cx - radius; x <= cx + radius; x++)
	{
		if (fx && x != cx)
		{
			continue;
		}
		for (int y = cy - radius; y <= cy + radius; y++)
		{
			if (fy && y != cy)
			{
				continue;
			}
			for (int z = cz - radius; z <= cz + radius; z++)
			{
				if (fz && z != cz)
				{
					continue;
				}
				int inside = 0;
				int outside = fill;
				for (int i = 0; i < 8; i++)
				{
					float dx = x + offsets[i][0] - cx;
					float dy = y + offsets[i][1] - cy;
					float dz = z + offsets[i][2] - cz;
					float d = sqrtf(dx * dx + dy * dy + dz * dz);
					if (d < radius)
					{
						inside = 1;
					}
					else
					{
						outside = 1;
					}
				}
				if (inside && outside)
				{
					builder_block(x, y, z, w);
				}
			}
		}
	}
}

void cylinder(Block *b1, Block *b2, int radius, int fill)
{
	if (b1->w != b2->w)
	{
		return;
	}
	int w = b1->w;
	int x1 = min(b1->x, b2->x);
	int y1 = min(b1->y, b2->y);
	int z1 = min(b1->z, b2->z);
	int x2 = max(b1->x, b2->x);
	int y2 = max(b1->y, b2->y);
	int z2 = max(b1->z, b2->z);
	int fx = x1 != x2;
	int fy = y1 != y2;
	int fz = z1 != z2;
	if (fx + fy + fz != 1)
	{
		return;
	}
	Block block = {x1, y1, z1, w};
	if (fx)
	{
		for (int x = x1; x <= x2; x++)
		{
			block.x = x;
			sphere(&block, radius, fill, 1, 0, 0);
		}
	}
	if (fy)
	{
		for (int y = y1; y <= y2; y++)
		{
			block.y = y;
			sphere(&block, radius, fill, 0, 1, 0);
		}
	}
	if (fz)
	{
		for (int z = z1; z <= z2; z++)
		{
			block.z = z;
			sphere(&block, radius, fill, 0, 0, 1);
		}
	}
}

void parse_command(const char *buffer, int forward)
{
	char username[128] = {0};
	char token[128] = {0};
	char server_addr[MAX_ADDR_LENGTH];
	int server_port = DEFAULT_PORT;
	char filename[MAX_PATH_LENGTH];
	int radius, count, xc, yc, zc;
	if (sscanf(buffer, "/identity %128s %128s", username, token) == 2)
	{
		db->authSet(username, token);
		add_message("Successfully imported identity token!");
		login();
	}
	else if (strcmp(buffer, "/logout") == 0)
	{
		db->authSelectNone();
		login();
	}
	else if (sscanf(buffer, "/login %128s", username) == 1)
	{
		if (db->authSelect(username))
		{
			login();
		}
		else
		{
			add_message("Unknown username.");
		}
	}
	else if (sscanf(buffer,
					"/online %128s %d", server_addr, &server_port) >= 1)
	{
		g->mode_changed = 1;
		g->mode = MODE_ONLINE;
		strncpy(g->server_addr, server_addr, MAX_ADDR_LENGTH);
		g->server_port = server_port;
		snprintf(g->db_path, MAX_PATH_LENGTH,
				 "cache.%s.%d.db", g->server_addr, g->server_port);
	}
	else if (sscanf(buffer, "/offline %128s", filename) == 1)
	{
		g->mode_changed = 1;
		g->mode = MODE_OFFLINE;
		snprintf(g->db_path, MAX_PATH_LENGTH, "%s.db", filename);
	}
	else if (strcmp(buffer, "/offline") == 0)
	{
		g->mode_changed = 1;
		g->mode = MODE_OFFLINE;
		snprintf(g->db_path, MAX_PATH_LENGTH, "%s", g->config->getDbPath().c_str());
	}
	else if (sscanf(buffer, "/view %d", &radius) == 1)
	{
		if (radius >= 1 && radius <= 24)
		{
			g->create_radius = radius;
			g->render_radius = radius;
			g->delete_radius = radius + 4;
		}
		else
		{
			add_message("Viewing distance must be between 1 and 24.");
		}
	}
	else if (strcmp(buffer, "/copy") == 0)
	{
		copy();
	}
	else if (strcmp(buffer, "/paste") == 0)
	{
		paste();
	}
	else if (strcmp(buffer, "/tree") == 0)
	{
		tree(&g->block0);
	}
	else if (sscanf(buffer, "/array %d %d %d", &xc, &yc, &zc) == 3)
	{
		array(&g->block1, &g->block0, xc, yc, zc);
	}
	else if (sscanf(buffer, "/array %d", &count) == 1)
	{
		array(&g->block1, &g->block0, count, count, count);
	}
	else if (strcmp(buffer, "/fcube") == 0)
	{
		cube(&g->block0, &g->block1, 1);
	}
	else if (strcmp(buffer, "/cube") == 0)
	{
		cube(&g->block0, &g->block1, 0);
	}
	else if (sscanf(buffer, "/fsphere %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 1, 0, 0, 0);
	}
	else if (sscanf(buffer, "/sphere %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 0, 0, 0, 0);
	}
	else if (sscanf(buffer, "/fcirclex %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 1, 1, 0, 0);
	}
	else if (sscanf(buffer, "/circlex %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 0, 1, 0, 0);
	}
	else if (sscanf(buffer, "/fcircley %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 1, 0, 1, 0);
	}
	else if (sscanf(buffer, "/circley %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 0, 0, 1, 0);
	}
	else if (sscanf(buffer, "/fcirclez %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 1, 0, 0, 1);
	}
	else if (sscanf(buffer, "/circlez %d", &radius) == 1)
	{
		sphere(&g->block0, radius, 0, 0, 0, 1);
	}
	else if (sscanf(buffer, "/fcylinder %d", &radius) == 1)
	{
		cylinder(&g->block0, &g->block1, radius, 1);
	}
	else if (sscanf(buffer, "/cylinder %d", &radius) == 1)
	{
		cylinder(&g->block0, &g->block1, radius, 0);
	}
	else if (forward)
	{
		client->talk(buffer);
	}
}

static void onKey(GLFWwindow *window, int key, int /*scancode*/, int action, int mods)
{
	int control = mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
	bool exclusive = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	if (action == GLFW_RELEASE)
		return;
	if (key == GLFW_KEY_BACKSPACE && g->typing)
	{
		int n = strlen(g->typing_buffer);
		if (n > 0)
			g->typing_buffer[n - 1] = '\0';
	}
	if (action != GLFW_PRESS)
		return;
	if (key == GLFW_KEY_ESCAPE)
	{
		if (g->typing)
			g->typing = 0;
		else if (exclusive)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (key == GLFW_KEY_ENTER)
	{
		if (g->typing)
		{
			if (mods & GLFW_MOD_SHIFT)
			{
				int n = strlen(g->typing_buffer);
				if (n < MAX_TEXT_LENGTH - 1)
				{
					g->typing_buffer[n] = '\r';
					g->typing_buffer[n + 1] = '\0';
				}
			}
			else
			{
				g->typing = 0;
				if (g->typing_buffer[0] == g->config->getKeySign())
				{
					int x, y, z, face;
					if (hitTestFace(&g->players[0], &x, &y, &z, &face))
						set_sign(x, y, z, face, g->typing_buffer + 1);
				}
				else if (g->typing_buffer[0] == '/')
					parse_command(g->typing_buffer, 1);
				else
					client->talk(g->typing_buffer);
			}
		}
		else
		{
			if (control)
				onRightClick();
			else
				onLeftClick();
		}
	}
	if (control && key == 'V')
	{
		const char *clip = glfwGetClipboardString(window);
		if (g->typing)
		{
			g->suppress_char = 1;
			strncat(g->typing_buffer, clip, MAX_TEXT_LENGTH - strlen(g->typing_buffer) - 1);
		}
		else
		{
			parse_command(clip, 0);
		}
	}
	if (!g->typing)
	{
		if (key == g->config->getKeyFly())
			g->flying = !g->flying;
		if (key >= '1' && key <= '9')
			g->item_index = key - '1';
		if (key == '0')
			g->item_index = 9;
		if (key == g->config->getKeyItemNext())
			g->item_index = (g->item_index + 1) % World::item_count;
		if (key == g->config->getKeyItemPrev())
		{
			g->item_index = (g->item_index - 1 + World::item_count) % World::item_count;
		}
		if (key == g->config->getKeyObserve())
			g->observe1 = (g->observe1 + 1) % g->player_count;
		if (key == g->config->getKeyObserveInset())
			g->observe2 = (g->observe2 + 1) % g->player_count;
	}
}

static void onChar(GLFWwindow *window, unsigned int codepoint)
{
	if (g->suppress_char)
	{
		g->suppress_char = 0;
		return;
	}
	if (g->typing)
	{
		if (codepoint >= 32 && codepoint < 128)
		{
			char c = static_cast<char>(codepoint);
			int n = strlen(g->typing_buffer);
			if (n < MAX_TEXT_LENGTH - 1)
			{
				g->typing_buffer[n] = c;
				g->typing_buffer[n + 1] = '\0';
			}
		}
	}
	else
	{
		if (codepoint == static_cast<unsigned int>(g->config->getKeyChat()))
		{
			g->typing = 1;
			g->typing_buffer[0] = '\0';
		}
		if (codepoint == g->config->getKeyCommand())
		{
			g->typing = 1;
			g->typing_buffer[0] = '/';
			g->typing_buffer[1] = '\0';
		}
		if (codepoint == g->config->getKeySign())
		{
			g->typing = 1;
			g->typing_buffer[0] = g->config->getKeySign();
			g->typing_buffer[1] = '\0';
		}
	}
}

static void onScroll(GLFWwindow *window, double xdelta, double ydelta)
{
	static double ypos = 0;
	ypos += ydelta;
	if (ypos < -SCROLL_THRESHOLD)
	{
		g->item_index = (g->item_index + 1) % World::item_count;
		ypos = 0;
	}
	if (ypos > SCROLL_THRESHOLD)
	{
		g->item_index = (g->item_index - 1 + World::item_count) % World::item_count;
		ypos = 0;
	}
}

static void onMouseButton(GLFWwindow *window, int button, int action, int mods)
{
	int control = mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
	bool exclusive = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	if (action != GLFW_PRESS)
		return;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (exclusive)
		{
			if (control)
				onRightClick();
			else
				onLeftClick();
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (exclusive)
		{
			if (control)
				onLight();
			else
				onRightClick();
		}
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && exclusive)
		onMiddleClick();
}

// --------------------------------------------------------------
// Fonctions de gestion de la fenêtre et de la boucle principale
// --------------------------------------------------------------

static void createWindow()
{
	int window_width = g->config->getWindowWidth();
	int window_height = g->config->getWindowHeight();
	GLFWmonitor *monitor = nullptr;
	if (g->config->isFullscreen())
	{
		monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		window_width = mode->width;
		window_height = mode->height;
	}
	g->window = glfwCreateWindow(window_width, window_height, "MinePP", monitor, nullptr);
}

static void handleMouseInput()
{
	static double px = 0, py = 0;
	bool exclusive = glfwGetInputMode(g->window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	State *s = &g->players[0].state;
	if (exclusive)
	{
		double mx, my;
		glfwGetCursorPos(g->window, &mx, &my);
		float m = 0.0025f;
		if (px || py)
		{
			s->rx += (mx - px) * m;
			s->ry += (g->config->isInvertMouse() ? (my - py) : -(my - py)) * m;
			if (s->rx < 0)
				s->rx += Utils::radians(360);
			if (s->rx >= Utils::radians(360))
				s->rx -= Utils::radians(360);
			s->ry = max(s->ry, -Utils::radians(90));
			s->ry = min(s->ry, Utils::radians(90));
		}
		px = mx;
		py = my;
	}
	else
	{
		glfwGetCursorPos(g->window, &px, &py);
	}
}

static void handleMovement(double dt)
{
	static float dy = 0;
	State *s = &g->players[0].state;
	int sz = 0, sx = 0;
	if (!g->typing)
	{
		float m = dt * 1.0f;
		g->ortho = glfwGetKey(g->window, g->config->getKeyOrtho()) ? 64 : 0;
		g->fov = glfwGetKey(g->window, g->config->getKeyZoom()) ? 15 : 65;
		if (glfwGetKey(g->window, g->config->getKeyForward()))
			sz--;
		if (glfwGetKey(g->window, g->config->getKeyBackward()))
			sz++;
		if (glfwGetKey(g->window, g->config->getKeyLeft()))
			sx--;
		if (glfwGetKey(g->window, g->config->getKeyRight()))
			sx++;
		if (glfwGetKey(g->window, GLFW_KEY_LEFT))
			s->rx -= m;
		if (glfwGetKey(g->window, GLFW_KEY_RIGHT))
			s->rx += m;
		if (glfwGetKey(g->window, GLFW_KEY_UP))
			s->ry += m;
		if (glfwGetKey(g->window, GLFW_KEY_DOWN))
			s->ry -= m;
	}
	float vx, vy, vz;
	get_motion_vector(g->flying, sz, sx, s->rx, s->ry, &vx, &vy, &vz);
	if (!g->typing)
	{
		if (glfwGetKey(g->window, g->config->getKeyJump()))
		{
			if (g->flying)
				vy = 1;
			else if (dy == 0)
				dy = 8;
		}
	}
	float speed = g->flying ? 20 : 5;
	int estimate = roundf(std::sqrt(powf(vx * speed, 2) + powf(vy * speed + fabs(dy) * 2, 2) + powf(vz * speed, 2)) * dt * 8);
	int step = max(8, estimate);
	float ut = dt / step;
	vx *= ut * speed;
	vy *= ut * speed;
	vz *= ut * speed;
	for (int i = 0; i < step; i++)
	{
		if (g->flying)
			dy = 0;
		else
		{
			dy -= ut * 25;
			dy = max(dy, -250.0f);
		}
		s->x += vx;
		s->y += vy + dy * ut;
		s->z += vz;
		if (collide(2, &s->x, &s->y, &s->z))
			dy = 0;
	}
	if (s->y < 0)
		s->y = highestBlock(s->x, s->z) + 2;
}

static void parseBuffer(char *buffer)
{
	// Traitement des messages reçus depuis le serveur
	char *key;
	char *line = Utils::tokenize(buffer, "\n", &key);
	while (line)
	{
		// Traitement de commandes et mise à jour des états joueurs, blocs, etc.
		// (Les sscanf et appels aux fonctions sont repris du code original)
		int pid;
		float ux, uy, uz, urx, ury;
		if (sscanf(line, "U,%d,%f,%f,%f,%f,%f", &pid, &ux, &uy, &uz, &urx, &ury) == 6)
		{
			Player *me = &g->players[0];
			me->id = pid;
			g->players[0].state = {ux, uy, uz, urx, ury, static_cast<float>(glfwGetTime())};
			forceChunks(me);
			if (uy == 0)
				g->players[0].state.y = highestBlock(ux, uz) + 2;
		}
		// Traitement des autres types de messages (B, L, P, D, K, R, E, T, N, S)
		// (Les sscanf et appels aux fonctions sont repris du code original)
		// ...
		line = Utils::tokenize(nullptr, "\n", &key);
	}
}

static void resetModel()
{
	for (int i = 0; i < MAX_CHUNKS; i++)
	{
		g->chunks[i] = Chunk();
	}
	g->chunk_count = 0;
	g->chunk_count = 0;
	memset(g->players, 0, sizeof(Player) * MAX_PLAYERS);
	g->player_count = 0;
	g->observe1 = 0;
	g->observe2 = 0;
	g->flying = 0;
	g->item_index = 0;
	memset(g->typing_buffer, 0, sizeof(g->typing_buffer));
	g->typing = 0;
	memset(g->messages, 0, sizeof(g->messages));
	g->message_index = 0;
	g->day_length = g->config->getDayLength();
	glfwSetTime(g->day_length / 3.0);
	g->time_changed = 1;
}

// --------------------------------------------------------------
// Boucle principale et initialisation
// --------------------------------------------------------------

GLuint gen_sky_buffer()
{
	float data[12288];
	Cube::makeSphere(data, 1, 3);
	return Utils::gen_buffer(sizeof(data), data);
}

int chunk_distance(Chunk *chunk, int p, int q)
{
	int dp = Utils::abs(chunk->p - p);
	int dq = Utils::abs(chunk->q - q);
	return max(dp, dq);
}

void delete_chunks()
{
	int count = g->chunk_count;
	State *s1 = &g->players->state;
	State *s2 = &(g->players + g->observe1)->state;
	State *s3 = &(g->players + g->observe2)->state;
	State *states[3] = {s1, s2, s3};
	for (int i = 0; i < count; i++)
	{
		Chunk *chunk = g->chunks + i;
		int shouldDelete = 1;
		for (int j = 0; j < 3; j++)
		{
			State *s = states[j];
			int p = chunked(s->x);
			int q = chunked(s->z);
			if (chunk_distance(chunk, p, q) < g->delete_radius)
			{
				shouldDelete = 0;
				break;
			}
		}
		if (shouldDelete)
		{
			chunk->map.clear();
			chunk->lights.clear();
			chunk->signs.clear();
			Utils::del_buffer(chunk->buffer);
			Utils::del_buffer(chunk->sign_buffer);
			Chunk *other = g->chunks + (--count);
			*chunk = *other;
		}
	}
	g->chunk_count = count;
}

void delete_all_chunks()
{
	for (int i = 0; i < g->chunk_count; i++)
	{
		Chunk *chunk = g->chunks + i;
		chunk->map.clear();
		chunk->lights.clear();
		chunk->signs.clear();
		Utils::del_buffer(chunk->buffer);
		Utils::del_buffer(chunk->sign_buffer);
	}
	g->chunk_count = 0;
}

int hit_test(
	int previous, float x, float y, float z, float rx, float ry,
	int *bx, int *by, int *bz)
{
	int result = 0;
	float best = 0;
	int p = chunked(x);
	int q = chunked(z);
	float vx, vy, vz;
	get_sight_vector(rx, ry, &vx, &vy, &vz);
	for (int i = 0; i < g->chunk_count; i++)
	{
		Chunk *chunk = g->chunks + i;
		if (chunk_distance(chunk, p, q) > 1)
		{
			continue;
		}
		int hx, hy, hz;
		int hw = _hitTest(&chunk->map, 8, previous,
						  x, y, z, vx, vy, vz, &hx, &hy, &hz);
		if (hw > 0)
		{
			float d = sqrtf(
				powf(hx - x, 2) + powf(hy - y, 2) + powf(hz - z, 2));
			if (best == 0 || d < best)
			{
				best = d;
				*bx = hx;
				*by = hy;
				*bz = hz;
				result = hw;
			}
		}
	}
	return result;
}

int hit_test_face(Player *player, int *x, int *y, int *z, int *face)
{
	State *s = &player->state;
	int w = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, x, y, z);
	if (World::isObstacle(w))
	{
		int hx, hy, hz;
		hit_test(1, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
		int dx = hx - *x;
		int dy = hy - *y;
		int dz = hz - *z;
		if (dx == -1 && dy == 0 && dz == 0)
		{
			*face = 0;
			return 1;
		}
		if (dx == 1 && dy == 0 && dz == 0)
		{
			*face = 1;
			return 1;
		}
		if (dx == 0 && dy == 0 && dz == -1)
		{
			*face = 2;
			return 1;
		}
		if (dx == 0 && dy == 0 && dz == 1)
		{
			*face = 3;
			return 1;
		}
		if (dx == 0 && dy == 1 && dz == 0)
		{
			int degrees = roundf(Utils::degrees(atan2f(s->x - hx, s->z - hz)));
			if (degrees < 0)
			{
				degrees += 360;
			}
			int top = ((degrees + 45) / 90) % 4;
			*face = 4 + top;
			return 1;
		}
	}
	return 0;
}

void draw_sign(Attrib *attrib, GLuint buffer, int length)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-8, -1024);
	drawTriangles3D_Text(attrib, buffer, length * 6);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void render_sign(Attrib *attrib, Player *player)
{
	if (!g->typing || g->typing_buffer[0] != g->config->getKeySign())
	{
		return;
	}
	int x, y, z, face;
	if (!hit_test_face(player, &x, &y, &z, &face))
	{
		return;
	}
	State *s = &player->state;
	float matrix[16];
	Utils::set_matrix_3d(
		matrix, g->width, g->height,
		s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib->sampler, 3);
	glUniform1i(attrib->extra1, 1);
	char text[Utils::MAX_SIGN_LENGTH];
	strncpy(text, g->typing_buffer + 1, Utils::MAX_SIGN_LENGTH);
	text[Utils::MAX_SIGN_LENGTH - 1] = '\0';
	GLfloat *data = Utils::malloc_faces(5, strlen(text));
	int length = _genSignBuffer(data, x, y, z, face, text);
	GLuint buffer = Utils::gen_faces(5, length, data);
	draw_sign(attrib, buffer, length);
	Utils::del_buffer(buffer);
}

void draw_cube(Attrib *attrib, GLuint buffer)
{
	drawItem(attrib, buffer, 36);
}

void draw_player(Attrib *attrib, Player *player)
{
	draw_cube(attrib, player->buffer);
}

void render_players(Attrib *attrib, Player *player)
{
	State *s = &player->state;
	float matrix[16];
	Utils::set_matrix_3d(
		matrix, g->width, g->height,
		s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform3f(attrib->camera, s->x, s->y, s->z);
	glUniform1i(attrib->sampler, 0);
	glUniform1f(attrib->timer, time_of_day());
	for (int i = 0; i < g->player_count; i++)
	{
		Player *other = g->players + i;
		if (other != player)
		{
			draw_player(attrib, other);
		}
	}
}

void render_sky(Attrib *attrib, Player *player, GLuint buffer)
{
	State *s = &player->state;
	float matrix[16];
	Utils::set_matrix_3d(
		matrix, g->width, g->height,
		0, 0, 0, s->rx, s->ry, g->fov, 0, g->render_radius);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib->sampler, 2);
	glUniform1f(attrib->timer, time_of_day());
	drawTriangles3D(attrib, buffer, 512 * 3);
}

int render_chunks(Attrib *attrib, Player *player)
{
	int result = 0;
	State *s = &player->state;
	ensureChunks(player);
	int p = chunked(s->x);
	int q = chunked(s->z);
	float light = get_daylight();
	float matrix[16];
	Utils::set_matrix_3d(
		matrix, g->width, g->height,
		s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
	float planes[6][4];
	Utils::frustum_planes(planes, g->render_radius, matrix);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform3f(attrib->camera, s->x, s->y, s->z);
	glUniform1i(attrib->sampler, 0);
	glUniform1i(attrib->extra1, 2);
	glUniform1f(attrib->extra2, light);
	glUniform1f(attrib->extra3, g->render_radius * CHUNK_SIZE);
	glUniform1i(attrib->extra4, g->ortho);
	glUniform1f(attrib->timer, time_of_day());
	for (int i = 0; i < g->chunk_count; i++)
	{
		Chunk *chunk = g->chunks + i;
		if (chunk_distance(chunk, p, q) > g->render_radius)
		{
			continue;
		}
		if (!chunkVisible(
				planes, chunk->p, chunk->q, chunk->miny, chunk->maxy))
		{
			continue;
		}
		drawChunk(attrib, chunk);
		result += chunk->faces;
	}
	return result;
}

void draw_signs(Attrib *attrib, Chunk *chunk)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-8, -1024);
	drawTriangles3D_Text(attrib, chunk->sign_buffer, chunk->sign_faces * 6);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void render_signs(Attrib *attrib, Player *player)
{
	State *s = &player->state;
	int p = chunked(s->x);
	int q = chunked(s->z);
	float matrix[16];
	Utils::set_matrix_3d(
		matrix, g->width, g->height,
		s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
	float planes[6][4];
	Utils::frustum_planes(planes, g->render_radius, matrix);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib->sampler, 3);
	glUniform1i(attrib->extra1, 1);
	for (int i = 0; i < g->chunk_count; i++)
	{
		Chunk *chunk = g->chunks + i;
		if (chunk_distance(chunk, p, q) > g->sign_radius)
		{
			continue;
		}
		if (!chunkVisible(
				planes, chunk->p, chunk->q, chunk->miny, chunk->maxy))
		{
			continue;
		}
		draw_signs(attrib, chunk);
	}
}

GLuint gen_wireframe_buffer(float x, float y, float z, float n)
{
	float data[72];
	Cube::makeCubeWireframe(data, x, y, z, n);
	return Utils::gen_buffer(sizeof(data), data);
}

void render_wireframe(Attrib *attrib, Player *player)
{
	State *s = &player->state;
	float matrix[16];
	Utils::set_matrix_3d(
		matrix, g->width, g->height,
		s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
	int hx, hy, hz;
	int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
	if (World::isObstacle(hw))
	{
		glUseProgram(attrib->program);
		glLineWidth(1);
		glEnable(GL_COLOR_LOGIC_OP);
		glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
		GLuint wireframe_buffer = gen_wireframe_buffer(hx, hy, hz, 0.53);
		drawLines(attrib, wireframe_buffer, 3, 24);
		Utils::del_buffer(wireframe_buffer);
		glDisable(GL_COLOR_LOGIC_OP);
	}
}

void render_crosshairs(Attrib *attrib)
{
	float matrix[16];
	Utils::set_matrix_2d(matrix, g->width, g->height);
	glUseProgram(attrib->program);
	glLineWidth(4 * g->scale);
	glEnable(GL_COLOR_LOGIC_OP);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	GLuint crosshair_buffer = gen_crosshair_buffer();
	drawLines(attrib, crosshair_buffer, 2, 4);
	Utils::del_buffer(crosshair_buffer);
	glDisable(GL_COLOR_LOGIC_OP);
}

GLuint gen_plant_buffer(float x, float y, float z, float n, int w)
{
	GLfloat *data = Utils::malloc_faces(10, 4);
	float ao = 0;
	float light = 1;
	Cube::makePlant(data, ao, light, x, y, z, n, w, 45);
	return Utils::gen_faces(10, 4, data);
}

void draw_plant(Attrib *attrib, GLuint buffer)
{
	drawItem(attrib, buffer, 24);
}

GLuint gen_cube_buffer(float x, float y, float z, float n, int w)
{
	GLfloat *data = Utils::malloc_faces(10, 6);
	float ao[6][4] = {0};
	float light[6][4] = {
		{0.5, 0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5, 0.5}};
	Cube::makeCube(data, ao, light, 1, 1, 1, 1, 1, 1, x, y, z, n, w);
	return Utils::gen_faces(10, 6, data);
}

void render_item(Attrib *attrib)
{
	float matrix[16];
	Utils::set_matrix_item(matrix, g->width, g->height, g->scale);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform3f(attrib->camera, 0, 0, 5);
	glUniform1i(attrib->sampler, 0);
	glUniform1f(attrib->timer, time_of_day());
	int w = World::items[g->item_index];
	if (World::isPlant(w))
	{
		GLuint buffer = gen_plant_buffer(0, 0, 0, 0.5, w);
		draw_plant(attrib, buffer);
		Utils::del_buffer(buffer);
	}
	else
	{
		GLuint buffer = gen_cube_buffer(0, 0, 0, 0.5, w);
		draw_cube(attrib, buffer);
		Utils::del_buffer(buffer);
	}
}

GLuint gen_text_buffer(float x, float y, float n, char *text)
{
	int length = strlen(text);
	GLfloat *data = Utils::malloc_faces(4, length);
	for (int i = 0; i < length; i++)
	{
		Cube::makeCharacter(data + i * 24, x, y, n / 2, n, text[i]);
		x += n;
	}
	return Utils::gen_faces(4, length, data);
}

void render_text(
	Attrib *attrib, int justify, float x, float y, float n, char *text)
{
	float matrix[16];
	Utils::set_matrix_2d(matrix, g->width, g->height);
	glUseProgram(attrib->program);
	glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
	glUniform1i(attrib->sampler, 1);
	glUniform1i(attrib->extra1, 0);
	int length = strlen(text);
	x -= n * justify * (length - 1) / 2;
	GLuint buffer = gen_text_buffer(x, y, n, text);
	drawText(attrib, buffer, length);
	Utils::del_buffer(buffer);
}
int main(int argc, char **argv)
{
	// INITIALISATION
	curl_global_init(CURL_GLOBAL_DEFAULT);
	srand(static_cast<unsigned int>(time(NULL)));
	rand();

	// INITIALISATION DE GLFW
	if (!glfwInit())
		return -1;
	createWindow();
	if (!g->window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(g->window);
	glfwSwapInterval(g->config->isVSync() ? 1 : 0);
	glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(g->window, onKey);
	glfwSetCharCallback(g->window, onChar);
	glfwSetMouseButtonCallback(g->window, onMouseButton);
	glfwSetScrollCallback(g->window, onScroll);

	if (glewInit() != GLEW_OK)
		return -1;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glLogicOp(GL_INVERT);
	glClearColor(0, 0, 0, 1);

	// CHARGEMENT DES TEXTURES
	GLuint texture, font, sky, signTex;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	Utils::load_png_texture("/home/denis/Documents/MinePP_new/textures/texture.png");

	glGenTextures(1, &font);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, font);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	Utils::load_png_texture("/home/denis/Documents/MinePP_new/textures/font.png");

	glGenTextures(1, &sky);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sky);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	Utils::load_png_texture("/home/denis/Documents/MinePP_new/textures/sky.png");

	glGenTextures(1, &signTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, signTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	Utils::load_png_texture("/home/denis/Documents/MinePP_new/textures/sign.png");

	// CHARGEMENT DES SHADERS et création des Attrib
	Attrib blockAttrib = {0}, lineAttrib = {0}, textAttrib = {0}, skyAttrib = {0};
	Attrib line_attrib = {0};
	Attrib text_attrib = {0};
	Attrib sky_attrib = {0};
	GLuint program;
	// (Les appels à DB::Utils::load_program ou équivalent doivent être faits via les modules shader)
	// Par exemple :
	program = Utils::load_program(
		"/home/denis/Documents/MinePP_new/shaders/block_vertex.glsl", "/home/denis/Documents/MinePP_new/shaders/block_fragment.glsl");
	blockAttrib.program = program;
	blockAttrib.position = glGetAttribLocation(program, "position");
	blockAttrib.normal = glGetAttribLocation(program, "normal");
	blockAttrib.uv = glGetAttribLocation(program, "uv");
	blockAttrib.matrix = glGetUniformLocation(program, "matrix");
	blockAttrib.sampler = glGetUniformLocation(program, "sampler");
	blockAttrib.extra1 = glGetUniformLocation(program, "sky_sampler");
	blockAttrib.extra2 = glGetUniformLocation(program, "daylight");
	blockAttrib.extra3 = glGetUniformLocation(program, "fog_distance");
	blockAttrib.extra4 = glGetUniformLocation(program, "ortho");
	blockAttrib.camera = glGetUniformLocation(program, "camera");
	blockAttrib.timer = glGetUniformLocation(program, "timer");

	program = Utils::load_program(
		"/home/denis/Documents/MinePP_new/shaders/line_vertex.glsl", "/home/denis/Documents/MinePP_new/shaders/line_fragment.glsl");
	line_attrib.program = program;
	line_attrib.position = glGetAttribLocation(program, "position");
	line_attrib.matrix = glGetUniformLocation(program, "matrix");

	program = Utils::load_program(
		"/home/denis/Documents/MinePP_new/shaders/text_vertex.glsl", "/home/denis/Documents/MinePP_new/shaders/text_fragment.glsl");
	text_attrib.program = program;
	text_attrib.position = glGetAttribLocation(program, "position");
	text_attrib.uv = glGetAttribLocation(program, "uv");
	text_attrib.matrix = glGetUniformLocation(program, "matrix");
	text_attrib.sampler = glGetUniformLocation(program, "sampler");
	text_attrib.extra1 = glGetUniformLocation(program, "is_sign");

	program = Utils::load_program(
		"/home/denis/Documents/MinePP_new/shaders/sky_vertex.glsl", "/home/denis/Documents/MinePP_new/shaders/sky_fragment.glsl");
	sky_attrib.program = program;
	sky_attrib.position = glGetAttribLocation(program, "position");
	sky_attrib.normal = glGetAttribLocation(program, "normal");
	sky_attrib.uv = glGetAttribLocation(program, "uv");
	sky_attrib.matrix = glGetUniformLocation(program, "matrix");
	sky_attrib.sampler = glGetUniformLocation(program, "sampler");
	sky_attrib.timer = glGetUniformLocation(program, "timer");

	// (Initialisation similaire pour lineAttrib, textAttrib et skyAttrib)

	// GESTION DES ARGUMENTS DE LIGNE DE COMMANDE
	if (argc == 2 || argc == 3)
	{
		g->mode = MODE_ONLINE;
		strncpy(g->server_addr, argv[1], MAX_ADDR_LENGTH);
		g->server_port = (argc == 3) ? atoi(argv[2]) : DEFAULT_PORT;
		snprintf(g->db_path, MAX_PATH_LENGTH, "cache.%s.%d.db", g->server_addr, g->server_port);
	}
	else
	{
		g->mode = MODE_OFFLINE;
		snprintf(g->db_path, MAX_PATH_LENGTH, "%s", g->config->getDbPath().c_str());
	}
	g->create_radius = g->config->getCreateChunkRadius();
	g->render_radius = g->config->getRenderChunkRadius();
	g->delete_radius = g->config->getDeleteChunkRadius();
	g->sign_radius = g->config->getRenderSignRadius();

	// INITIALISATION DES WORKERS
	for (int i = 0; i < WORKERS; i++)
	{
		Worker *worker = &g->workers[i];
		worker->index = i;
		worker->state = WORKER_IDLE;
		// Initialisation des mutex et condition_variable
		// (std::mutex et std::condition_variable sont automatiquement initialisés)
		worker->thrd = std::thread(workerRun, worker);
	}

	// BOUCLE PRINCIPALE
	int running = 1;
	if (g->mode == MODE_OFFLINE || g->config->useCache())
	{
		db->enable();
		if (db->init(g->db_path) != 0)
			return -1;
		if (g->mode == MODE_ONLINE)
			db->deleteAllSigns();
	}
	if (g->mode == MODE_ONLINE)
	{
		client->enable();
		client->connect(g->server_addr, g->server_port);
		client->start();
		client->version(1);
		login();
	}
	resetModel();
	Utils::FPS fps = {0, 0, 0};
	double last_commit = glfwGetTime();
	double last_update = glfwGetTime();
	GLuint skyBuffer = gen_sky_buffer();

	Player *me = &g->players[0];
	me->id = 0;
	me->name[0] = '\0';
	me->buffer = 0;
	g->player_count = 1;
	if (!db->loadState(me->state.x, me->state.y, me->state.z, me->state.rx, me->state.ry))
		me->state.y = highestBlock(me->state.x, me->state.z) + 2;

	double previous = glfwGetTime();
	while (!glfwWindowShouldClose(g->window) && !g->mode_changed)
	{
		g->scale = get_scale_factor();
		glfwGetFramebufferSize(g->window, &g->width, &g->height);
		glViewport(0, 0, g->width, g->height);

		if (g->time_changed)
		{
			g->time_changed = 0;
			last_commit = glfwGetTime();
			last_update = glfwGetTime();
			memset(&fps, 0, sizeof(fps));
		}
		Utils::update_fps(fps);
		double now = glfwGetTime();
		double dt = now - previous;
		dt = min(dt, 0.2);
		dt = max(dt, 0.0);
		previous = now;

		handleMouseInput();
		handleMovement(dt);
		// std::string temp = client->recv();
		// char buffer[temp.length() + 1];
		// strcpy(buffer, client->recv().c_str());
		// if (buffer)
		// {
		// 	parseBuffer(buffer);
		// 	free(buffer);
		// }
		std::string received = client->recv();
		if (!received.empty())
		{
			// Si parseBuffer prend un char*, vous pouvez utiliser received.data() (attention : data() retourne un pointeur constant)
			// Vous pouvez soit modifier parseBuffer pour accepter un std::string ou alors créer une copie dynamique.
			char *buffer = new char[received.length() + 1];
			std::strcpy(buffer, received.c_str());
			parseBuffer(buffer);
			delete[] buffer;
		}
		if (now - last_commit > g->config->getCommitInterval())
		{
			last_commit = now;
			db->commit();
		}
		if (now - last_update > 0.1)
		{
			last_update = now;
			client->position(me->state.x, me->state.y, me->state.z, me->state.rx, me->state.ry);
		}
		g->observe1 %= g->player_count;
		g->observe2 %= g->player_count;
		delete_chunks();
		Utils::del_buffer(me->buffer);
		me->buffer = gen_player_buffer(me->state.x, me->state.y, me->state.z, me->state.rx, me->state.ry);
		for (int i = 1; i < g->player_count; i++)
			interpolatePlayer(&g->players[i]);
		Player *player = &g->players[g->observe1];

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_sky(&skyAttrib, player, skyBuffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		int face_count = render_chunks(&blockAttrib, player);
		render_signs(&textAttrib, player);
		render_sign(&textAttrib, player);
		render_players(&blockAttrib, player);
		if (g->config->showWireframe())
			render_wireframe(&lineAttrib, player);

		glClear(GL_DEPTH_BUFFER_BIT);
		if (g->config->showCrosshairs())
			render_crosshairs(&lineAttrib);
		if (g->config->showItem())
			render_item(&blockAttrib);
		char textBuffer[1024];
		float ts = 12 * g->scale;
		float tx = ts / 2;
		float ty = g->height - ts;
		if (g->config->showInfoText())
		{
			int hour = static_cast<int>(time_of_day() * 24);
			char am_pm = (hour < 12) ? 'a' : 'p';
			hour = hour % 12;
			hour = (hour == 0) ? 12 : hour;
			snprintf(textBuffer, sizeof(textBuffer),
					 "(%d, %d) (%.2f, %.2f, %.2f) [%d, %d, %d] %d%cm %dfps",
					 chunked(me->state.x), chunked(me->state.z),
					 me->state.x, me->state.y, me->state.z,
					 g->player_count, g->chunk_count, face_count * 2,
					 hour, am_pm, fps.fps);
			render_text(&textAttrib, ALIGN_LEFT, tx, ty, ts, textBuffer);
			ty -= ts * 2;
		}
		if (g->config->showChatText())
		{
			for (int i = 0; i < 4; i++)
			{
				int index = (g->message_index + i) % 4;
				if (strlen(g->messages[index]) > 0)
				{
					render_text(&textAttrib, ALIGN_LEFT, tx, ty, ts, g->messages[index]);
					ty -= ts * 2;
				}
			}
		}
		if (g->typing)
		{
			snprintf(textBuffer, sizeof(textBuffer), "> %s", g->typing_buffer);
			render_text(&textAttrib, ALIGN_LEFT, tx, ty, ts, textBuffer);
			ty -= ts * 2;
		}
		if (g->config->showPlayerNames())
		{
			if (player != me)
				render_text(&textAttrib, ALIGN_CENTER, g->width / 2, ts, ts, player->name);
			Player *other = playerCrosshair(player);
			if (other)
				render_text(&textAttrib, ALIGN_CENTER, g->width / 2, g->height / 2 - ts - 24, ts, other->name);
		}
		// Affichage en picture-in-picture si observe2 est actif
		if (g->observe2)
		{
			player = &g->players[g->observe2];
			int pw = 256 * g->scale, ph = 256 * g->scale, offset = 32 * g->scale, pad = 3 * g->scale;
			int sw = pw + pad * 2, sh = ph + pad * 2;
			glEnable(GL_SCISSOR_TEST);
			glScissor(g->width - sw - offset + pad, offset - pad, sw, sh);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_SCISSOR_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			glViewport(g->width - pw - offset, offset, pw, ph);
			g->width = pw;
			g->height = ph;
			g->ortho = 0;
			g->fov = 65;
			render_sky(&skyAttrib, player, skyBuffer);
			glClear(GL_DEPTH_BUFFER_BIT);
			render_chunks(&blockAttrib, player);
			render_signs(&textAttrib, player);
			render_players(&blockAttrib, player);
			glClear(GL_DEPTH_BUFFER_BIT);
			if (g->config->showPlayerNames())
				render_text(&textAttrib, ALIGN_CENTER, pw / 2, ts, ts, player->name);
		}

		glfwSwapBuffers(g->window);
		glfwPollEvents();
		if (glfwWindowShouldClose(g->window))
		{
			running = 0;
			break;
		}
		if (g->mode_changed)
		{
			g->mode_changed = 0;
			break;
		}
	}
	db->saveState(me->state.x, me->state.y, me->state.z, me->state.rx, me->state.ry);
	db->close();
	db->disable();
	client->stop();
	client->disable();
	Utils::del_buffer(skyBuffer);
	delete_all_chunks();
	deleteAllPlayers();
	glfwTerminate();
	curl_global_cleanup();
	return 0;
}
