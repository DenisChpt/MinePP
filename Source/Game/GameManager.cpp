#include "GameManager.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Rendering/Shader.hpp"     // si tu as ta classe Shader
#include "ResourceManager/AtlasGenerator.hpp"
#include "Entities/Character.hpp"
#include "Input/InputManager.hpp"
#include "Rendering/Camera.hpp"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

namespace Game {

static GLuint compileShader(GLenum type, const std::string &source);
static GLuint createShaderProgram(const std::string &vertSrc, const std::string &fragSrc);

GameManager::GameManager()
	: m_shaderProgram(0)
	, m_sunAngle(0.0f)
{
}

GameManager::~GameManager()
{
}

void GameManager::init()
{
	std::cout << "[GameManager] Init\n";

	// 1) Charger shaders, atlas, etc.
	if(!initResources()) {
		std::cerr << "[GameManager] initResources failed\n";
		return;
	}

	// 2) Créer un perso
	m_player = std::make_unique<Character>(glm::vec3(0,25,0));

	// 3) Générer le monde
	WorldGenerator generator(12345); // seed
	m_chunks = generator.generateWorld(m_atlas);
}

bool GameManager::initResources()
{
	// Exemple: On compile un shader simpliste
	std::string vertSrc = R"(
		#version 330 core
		layout(location=0) in vec3 aPos;
		layout(location=1) in vec2 aTexCoord;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		out vec2 TexCoord;

		void main() {
			gl_Position = projection * view * model * vec4(aPos,1.0);
			TexCoord = aTexCoord;
		}
	)";

	std::string fragSrc = R"(
		#version 330 core
		in vec2 TexCoord;
		out vec4 FragColor;

		uniform sampler2D texture1;

		void main(){
			FragColor = texture(texture1, TexCoord);
		}
	)";

	m_shaderProgram = createShaderProgram(vertSrc, fragSrc);
	if(!m_shaderProgram) {
		return false;
	}

	// Générer l’atlas 1024x1024
	AtlasGenerator atlasGen(1024,1024);
	if(!atlasGen.generateAtlas(
		 "/home/denis/Documents/Meinkraft/Assets/Textures/Config/textures.json", // chemin
		 m_atlas))
	{
		std::cerr << "[GameManager] Echec generateAtlas\n";
		return false;
	}
	std::cout << "[GameManager] Atlas loaded, textureID=" << m_atlas.textureID << "\n";

	std::string vs2D = R"(
		#version 330 core
		layout(location=0) in vec2 aPos;
		layout(location=1) in vec2 aTexCoord;

		uniform mat4 uProjection;

		out vec2 vTexCoord;

		void main() {
			gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
			vTexCoord = aTexCoord;
		}
	)";

	// Fragment shader
	std::string fs2D = R"(
		#version 330 core
		in vec2 vTexCoord;
		out vec4 FragColor;

		uniform sampler2D uTexture;

		void main() {
			FragColor = texture(uTexture, vTexCoord);
		}
	)";

	m_2DShaderProgram = createShaderProgram(vs2D, fs2D);
	if (!m_2DShaderProgram) {
		std::cerr << "[GameManager] Failed to create 2D shader program.\n";
		return false;
	}
	static const std::string wireVertexShader = R"(
		#version 330 core
		layout(location = 0) in vec3 aPos;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
			gl_Position = projection * view * model * vec4(aPos, 1.0);
		}
	)";

	static const std::string wireFragmentShader = R"(
		#version 330 core

		out vec4 FragColor;
		uniform vec4 highlightColor;

		void main()
		{
			FragColor = highlightColor;
		}
	)";

	// Puis compilation
	m_wireShaderProgram = createShaderProgram(wireVertexShader, wireFragmentShader);
	if(!m_wireShaderProgram) {
		std::cerr << "Failed to compile wireframe highlight shader\n";
		return false; 
	}
	return true;
}

void GameManager::update(float dt, const Engine::Input::InputManager& input, const Engine::Rendering::Camera& camera)
{
	m_sunAngle += dt * 0.01f;
	if (m_player) {
		float speed = 25.0f;
		// On prend la direction "front" de la caméra
		glm::vec3 front = camera.getFront();
		// On calcule la direction "right"
		glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));

		// On check les touches
		if (input.isKeyDown(SDL_SCANCODE_W)) {
			m_player->move(front * speed * dt);
		}
		if (input.isKeyDown(SDL_SCANCODE_S)) {
			m_player->move(-front * speed * dt);
		}
		if (input.isKeyDown(SDL_SCANCODE_A)) {
			m_player->move(-right * speed * dt);
		}
		if (input.isKeyDown(SDL_SCANCODE_D)) {
			m_player->move(right * speed * dt);
		}
	}

	updateHoveredBlock(camera);

	handleBlockInteraction(input, camera);
}

void GameManager::renderSun(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos)
{
	// 1) Calcul de la position du soleil
	float radius = 300.0f; // distance du centre
	float sunX = cos(m_sunAngle) * radius;
	float sunZ = sin(m_sunAngle) * radius;
	float sunY = 150.0f;   // hauteur
	glm::vec3 sunPos(sunX, sunY, sunZ);

	// 2) Préparer un “billboard” qui fait toujours face à la caméra
	//    On va construire une matrice model = translate(sunPos) * billboardRotation * scale
	glm::vec3 look = glm::normalize(cameraPos - sunPos);
	glm::vec3 up   = glm::vec3(0,1,0);
	glm::vec3 right= glm::normalize(glm::cross(up, look));
	up             = glm::normalize(glm::cross(look, right)); // orthonormal

	// Construisons une matrice 3x3 (column major)
	glm::mat4 rot(1.0f);
	rot[0] = glm::vec4(right, 0.0f);
	rot[1] = glm::vec4(up,    0.0f);
	rot[2] = glm::vec4(look,  0.0f);

	// Appliquer translation + rotation + échelle
	glm::mat4 model(1.0f);
	model = glm::translate(model, sunPos);
	model = model * rot;
	float sunSize = 30.0f;  // taille du soleil
	model = glm::scale(model, glm::vec3(sunSize)); 

	// 3) On utilise un shader “3D sprite” (peut être identique à crosshair 2D 
	//    sauf qu’on fait view/proj en 3D). Pour simplifier, on peut réutiliser un 
	//    mini-shader texturé. Ex: m_shaderProgram (ou un “m_sprite3DShader”).
	glUseProgram(m_shaderProgram);

	// Envoi model/view/proj
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(proj));

	// On récupère la zone “sun” dans l’atlas
	auto it = m_atlas.mapping.find("sun");
	if (it == m_atlas.mapping.end()) {
		std::cerr << "[GameManager] 'sun' not found in atlas.\n";
		return;
	}
	const AtlasEntry &entry = it->second;
	float atlasW = float(m_atlas.atlasWidth);
	float atlasH = float(m_atlas.atlasHeight);

	float u_min = entry.x / atlasW;
	float v_min = entry.y / atlasH;
	float u_max = (entry.x + entry.width) / atlasW;
	float v_max = (entry.y + entry.height) / atlasH;

	// 4) Construire un quad local de [-0.5..+0.5] x [-0.5..+0.5], z=0
	//    => 2 triangles => 6 vertices
	float vertices[] = {
		//  x, y, z,    u, v
		-0.5f, +0.5f, 0.0f,   u_min, v_min,
		+0.5f, +0.5f, 0.0f,   u_max, v_min,
		+0.5f, -0.5f, 0.0f,   u_max, v_max,

		+0.5f, -0.5f, 0.0f,   u_max, v_max,
		-0.5f, -0.5f, 0.0f,   u_min, v_max,
		-0.5f, +0.5f, 0.0f,   u_min, v_min
	};

	// 5) Créer un VAO/VBO et dessiner
	GLuint vao=0,vbo=0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// location=0 => pos (3 floats)
	// location=1 => texCoord (2 floats)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	// Bind la texture atlas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_atlas.textureID);
	glUniform1i(glGetUniformLocation(m_shaderProgram,"texture1"), 0);

	// On dessine en “alpha-blend” ? => 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// On peut désactiver le depth test si on veut toujours voir le sun
	// glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Nettoyage
	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	// glEnable(GL_DEPTH_TEST); // si on l'avait désactivé
	glDisable(GL_BLEND);
}

void GameManager::renderBlockHighlight(const glm::mat4& view, const glm::mat4& proj)
{
	if(!m_hoveredBlock.valid) return;

	// 1) coords world
	int wx = m_hoveredBlock.chunkX * World::CHUNK_SIZE + m_hoveredBlock.localX;
	int wy = m_hoveredBlock.localY;
	int wz = m_hoveredBlock.chunkZ * World::CHUNK_SIZE + m_hoveredBlock.localZ;

	// Centre
	glm::vec3 center(wx, wy, wz);

	// 2) On utilise un shader distinct (m_wireShaderProgram)
	glUseProgram(m_wireShaderProgram);

	// Matière
	GLint locColor = glGetUniformLocation(m_wireShaderProgram, "highlightColor");
	glUniform4f(locColor, 1.0f, 1.0f, 0.0f, 1.0f); // Jaune

	// Matrices
	glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
	glUniformMatrix4fv(glGetUniformLocation(m_wireShaderProgram,"model"),1,GL_FALSE,glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(m_wireShaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_wireShaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(proj));

	// 3) On désactive le cull face ou on laisse ?
	// glDisable(GL_CULL_FACE);

	// 4) On applique un offset
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

	GLuint vao=0,vbo=0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 12 arêtes × 2 = 24 points
	static const float edges[] = {
		// 2 vertices par arête => 24 points
		// “top face” edges
		0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
		// “bottom face” edges
		0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,
		// “vertical” edges
		0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(edges), edges, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);

	// 6) on dessine en LINES
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, 24);

	// 7) nettoyer
	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// glEnable(GL_CULL_FACE); // si tu veux le remettre
}


void GameManager::render(const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight)
{
	// Utiliser le shader
	glUseProgram(m_shaderProgram);

	// model = identity (car on stocke positions absolues dans nos vertices)
	glm::mat4 model(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram,"view"), 1,GL_FALSE,glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram,"projection"), 1,GL_FALSE,glm::value_ptr(projection));

	// Lier la texture atlas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_atlas.textureID);
	glUniform1i(glGetUniformLocation(m_shaderProgram,"texture1"), 0);

	// Dessiner les chunks
	for (auto &chunk : m_chunks) {
		chunk->render();
	}

	if(m_hoveredBlock.valid)
		renderBlockHighlight(view, projection);

	renderCrosshairTexture(screenWidth, screenHeight);

	renderSun(view, projection, m_player->getPosition());
	m_player->render(m_atlas);

	// HUD ?
}

void GameManager::shutdown()
{
	std::cout << "[GameManager] Shutdown\n";
	if(m_shaderProgram) {
		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;
	}
}

void GameManager::handleBlockInteraction(const Engine::Input::InputManager& input, const Engine::Rendering::Camera& camera)
{
	// Si le bouton gauche de la souris vient d'être pressé
	if (!input.isMouseButtonPressed(SDL_BUTTON_LEFT))
		return;

	// On utilise le bloc déjà “hovered” (raycast continu)
	if (!m_hoveredBlock.valid) {
		std::cout << "[GameManager] No hovered block to remove.\n";
		return;
	}

	// Récup infos
	auto chunk = m_hoveredBlock.chunk;
	int localX = m_hoveredBlock.localX;
	int localY = m_hoveredBlock.localY;
	int localZ = m_hoveredBlock.localZ;

	if (!chunk) {
		std::cout << "[GameManager] hoveredBlock has no chunk?\n";
		return;
	}
	
	// Supprime le bloc
	chunk->setBlock(localX, localY, localZ, BlockType::Air);
	chunk->generateMesh(m_atlas);

	std::cout << "[GameManager] Removed block at chunk (" << m_hoveredBlock.chunkX << ","
			  << m_hoveredBlock.chunkZ << ") local (" << localX << "," << localY 
			  << "," << localZ << ")\n";
}


void GameManager::updateHoveredBlock(const Engine::Rendering::Camera& camera)
{
	// On réinitialise d’abord
	m_hoveredBlock.valid = false;

	glm::vec3 rayOrigin = camera.getPosition();
	glm::vec3 rayDir = glm::normalize(camera.getFront());
	float maxDistance = 8.0f;
	float step = 0.005f;
	float dist = 0.0f;

	while(dist < maxDistance) {
		glm::vec3 pos = rayOrigin + rayDir * dist;
		int worldX = (int)std::floor(pos.x);
		int worldY = (int)std::floor(pos.y);
		int worldZ = (int)std::floor(pos.z);

		int cx = worldX / World::CHUNK_SIZE;
		int cz = worldZ / World::CHUNK_SIZE;

		// Trouve le chunk (m_chunks est un std::vector<std::shared_ptr<Chunk>>)
		std::shared_ptr<World::Chunk> foundChunk = nullptr;
		for(auto &c : m_chunks) {
			if(c->getChunkX() == cx && c->getChunkZ() == cz) {
				foundChunk = c; 
				break;
			}
		}
		if(foundChunk) {
			int localX = worldX % World::CHUNK_SIZE;
			int localZ = worldZ % World::CHUNK_SIZE;
			if(localX < 0) localX += World::CHUNK_SIZE;
			if(localZ < 0) localZ += World::CHUNK_SIZE;
			int localY = worldY;

			if(localY >= 0 && localY < World::CHUNK_HEIGHT) {
				BlockType t = foundChunk->getBlock(localX, localY, localZ);
				if(t != BlockType::Air && t != BlockType::Water) {
					// On a trouvé un bloc "solide"
					m_hoveredBlock.valid = true;
					m_hoveredBlock.chunk = foundChunk;
					m_hoveredBlock.chunkX = cx;
					m_hoveredBlock.chunkZ = cz;
					m_hoveredBlock.localX = localX;
					m_hoveredBlock.localY = localY;
					m_hoveredBlock.localZ = localZ;
					return; // on s’arrête au 1er bloc
				}
			}
		}
		dist += step;
	}
}

void GameManager::renderCrosshairTexture(int screenW, int screenH)
{
	// --- Activer le blending pour gérer la transparence ---
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 1) Utiliser le shader 2D
	glUseProgram(m_2DShaderProgram);

	// 2) Désactiver le depth test pour dessiner par-dessus
	glDisable(GL_DEPTH_TEST);

	// 3) Créer la matrice orthographique, 
	//    où (0,0) est en haut-gauche et (screenW, screenH) en bas-droite
	glm::mat4 ortho = glm::ortho(
		0.0f, static_cast<float>(screenW),
		static_cast<float>(screenH), 0.0f,
		-1.0f, 1.0f
	);

	// Envoyer cette matrice dans le shader
	GLint locProj = glGetUniformLocation(m_2DShaderProgram, "uProjection");
	glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(ortho));

	// 4) Récupérer les UV du crosshair dans l’atlas
	auto it = m_atlas.mapping.find("crosshair");
	if (it == m_atlas.mapping.end()) {
		std::cerr << "[GameManager] Crosshair texture not found in atlas.\n";
		return;
	}
	const AtlasEntry &entry = it->second;
	float atlasW = static_cast<float>(m_atlas.atlasWidth);
	float atlasH = static_cast<float>(m_atlas.atlasHeight);

	float u_min = entry.x / atlasW;
	float v_min = entry.y / atlasH;
	float u_max = (entry.x + entry.width) / atlasW;
	float v_max = (entry.y + entry.height) / atlasH;

	// 5) Dimensions du crosshair à l’écran (en pixels)
	float crossW = 32.0f;
	float crossH = 32.0f;

	// 6) Centre de l’écran
	float cx = screenW * 0.5f; 
	float cy = screenH * 0.5f;

	// 7) Coords du quad : on veut le centre de la texture à (cx, cy)
	//    => le coin supérieur-gauche est donc (cx - crossW/2, cy - crossH/2)
	float left   = cx - crossW * 0.5f;
	float right  = cx + crossW * 0.5f;
	float top    = cy - crossH * 0.5f;
	float bottom = cy + crossH * 0.5f;

	// 8) Vertices (2 triangles) : (x,y, u,v)
	float vertices[6 * 4] = {
		// Triangle 1
		left,   top,    u_min, v_min,
		right,  top,    u_max, v_min,
		right,  bottom, u_max, v_max,

		// Triangle 2
		right,  bottom, u_max, v_max,
		left,   bottom, u_min, v_max,
		left,   top,    u_min, v_min
	};

	// 9) Création du VAO/VBO pour dessiner le quad
	GLuint vao=0, vbo=0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position => location=0 (2 floats)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// UV => location=1 (2 floats)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);

	// 10) Bind la texture atlas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_atlas.textureID);
	GLint locTex = glGetUniformLocation(m_2DShaderProgram, "uTexture");
	glUniform1i(locTex, 0);

	// 11) Dessin
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// 12) Nettoyage
	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	// 13) Réactiver le depth test si nécessaire
	glEnable(GL_DEPTH_TEST);

	// (Optionnel) Désactiver le blending s’il ne sert plus
	// glDisable(GL_BLEND);
}

// ======================= STATIC UTILS =======================
static GLuint compileShader(GLenum type, const std::string &source)
{
	GLuint shader = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader,512,nullptr,infoLog);
		std::cerr << "[Shader] Compile error:\n" << infoLog << std::endl;
	}
	return shader;
}

static GLuint createShaderProgram(const std::string &vertSrc, const std::string &fragSrc)
{
	GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	GLint linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if(!linked) {
		char infoLog[512];
		glGetProgramInfoLog(prog,512,nullptr,infoLog);
		std::cerr << "[Shader] Link error:\n" << infoLog << std::endl;
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
	return prog;
}

} // namespace Game
