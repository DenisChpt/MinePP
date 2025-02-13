#include "Character.hpp"
#include <iostream>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

namespace Game {

Character::Character(const glm::vec3& position)
	: m_Position(position),
	  m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	  m_Up(glm::vec3(0.0f, 1.0f, 0.0f)),
	  m_Yaw(-90.0f),
	  m_Pitch(0.0f),
	  m_Speed(5.0f),
	  m_IsFlying(false)
{
	updateDirection();
}

Character::~Character() {
}

void Character::move(const glm::vec3& delta) {
	m_Position += delta;
	std::cout << "[Character] Move to ("
			  << m_Position.x << ", " << m_Position.y << ", " << m_Position.z << ")." << std::endl;
}

void Character::jump() {
	std::cout << "[Character] Jump!" << std::endl;
	m_Position.y += 1.0f;
}

void Character::toggleFlying() {
	m_IsFlying = !m_IsFlying;
	std::cout << "[Character] Flying mode: " << (m_IsFlying ? "ON" : "OFF") << std::endl;
}

void Character::update(float deltaTime) {
	// std::cout << "[Character] Update (" << deltaTime << "s)." << std::endl;
	// Vous pourrez ajouter ici la physique ou les animations.
}

glm::mat4 Character::getViewMatrix() const {
	return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void Character::addYaw(float offset) {
	m_Yaw += offset;
	updateDirection();
}

void Character::addPitch(float offset) {
	m_Pitch += offset;
	if (m_Pitch > 89.0f)
		m_Pitch = 89.0f;
	if (m_Pitch < -89.0f)
		m_Pitch = -89.0f;
	updateDirection();
}

void Character::updateDirection() {
	glm::vec3 front;
	front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	front.y = sin(glm::radians(m_Pitch));
	front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front = glm::normalize(front);
}

const glm::vec3& Character::getPosition() const {
	return m_Position;
}

/**
 * Rend le personnage complet.
 * On suppose que l'atlas contient la texture "character" (64×64).
 * Les coordonnées de chaque partie sont définies selon le CSV.
 */
void Character::render(const Atlas& atlas) {
	// Récupérer l'entrée pour "character" dans l'atlas.
	auto it = atlas.mapping.find("character");
	if (it == atlas.mapping.end()) {
		std::cerr << "[Character::render] Texture 'character' non trouvée dans l'atlas." << std::endl;
		return;
	}
	AtlasEntry charEntry = it->second;

	// Lambda pour calculer les coordonnées UV pour une sous-région définie dans le skin.
	auto computeUV = [&](int csvX, int csvY, int csvW, int csvH) -> glm::vec4 {
		float u_min = (charEntry.x + csvX) / float(atlas.atlasWidth);
		float v_min = (charEntry.y + csvY) / float(atlas.atlasHeight);
		float u_max = (charEntry.x + csvX + csvW) / float(atlas.atlasWidth);
		float v_max = (charEntry.y + csvY + csvH) / float(atlas.atlasHeight);
		return glm::vec4(u_min, v_min, u_max, v_max);
	};

	// Conversion : 1 pixel = 0.0625 unités (8 pixels = 0.5 unités, comme en Minecraft)
	float pixelToUnit = 0.0625f;

	// Définir les dimensions de chaque partie (en unités de jeu)
	glm::vec3 headSize(8 * pixelToUnit, 8 * pixelToUnit, 8 * pixelToUnit);       // 0.5 x 0.5 x 0.5
	glm::vec3 torsoSize(8 * pixelToUnit, 12 * pixelToUnit, 4 * pixelToUnit);      // 0.5 x 0.75 x 0.25
	glm::vec3 armSize(4 * pixelToUnit, 12 * pixelToUnit, 4 * pixelToUnit);        // 0.25 x 0.75 x 0.25
	glm::vec3 legSize(4 * pixelToUnit, 12 * pixelToUnit, 4 * pixelToUnit);        // 0.25 x 0.75 x 0.25

	// Positionnement relatif (on suppose que m_Position est le centre du torse)
	glm::vec3 torsoCenter = m_Position;
	glm::vec3 headCenter = m_Position + glm::vec3(0.0f, (torsoSize.y / 2 + headSize.y / 2), 0.0f);
	glm::vec3 leftArmCenter = m_Position + glm::vec3((torsoSize.x / 2 + armSize.x / 2), 0.0f, 0.0f);
	glm::vec3 rightArmCenter = m_Position + glm::vec3(-(torsoSize.x / 2 + armSize.x / 2), 0.0f, 0.0f);
	glm::vec3 leftLegCenter = m_Position + glm::vec3(0.125f, -(torsoSize.y / 2 + legSize.y / 2), 0.0f);
	glm::vec3 rightLegCenter = m_Position + glm::vec3(-0.125f, -(torsoSize.y / 2 + legSize.y / 2), 0.0f);

	// Calculer les UV pour chaque partie à partir du CSV :
	// Tête (Head)
	glm::vec4 headFrontUV   = computeUV(8 , 8 , 8, 8 );
	glm::vec4 headBackUV    = computeUV(24, 8 , 8, 8 );
	glm::vec4 headLeftUV    = computeUV(16, 8 , 8, 8 );
	glm::vec4 headRightUV   = computeUV(0 , 8 , 8, 8 );
	glm::vec4 headTopUV     = computeUV(8 , 0 , 8, 8 );
	glm::vec4 headBottomUV  = computeUV(16, 0 , 8, 8 );

	// Corps (Torso)
	glm::vec4 torsoFrontUV  = computeUV(20, 20, 8, 12);
	glm::vec4 torsoBackUV   = computeUV(32, 20, 8, 12);
	glm::vec4 torsoLeftUV   = computeUV(28, 20, 4, 12);
	glm::vec4 torsoRightUV  = computeUV(16, 20, 4, 12);
	glm::vec4 torsoTopUV    = computeUV(20, 16, 8, 4 );
	glm::vec4 torsoBottomUV = computeUV(28, 16, 8, 4 );

	// Bras gauche (Left arm)
	glm::vec4 lArmFrontUV   = computeUV(36, 52, 4, 12);
	glm::vec4 lArmBackUV    = computeUV(44, 52, 4, 12);
	glm::vec4 lArmLeftUV    = computeUV(40, 52, 4, 12);
	glm::vec4 lArmRightUV   = computeUV(32, 52, 4, 12);
	glm::vec4 lArmTopUV     = computeUV(36, 48, 4, 4 );
	glm::vec4 lArmBottomUV  = computeUV(40, 48, 4, 4 );

	// Bras droit (Right arm)
	glm::vec4 rArmFrontUV   = computeUV(44, 20, 4, 12);
	glm::vec4 rArmBackUV    = computeUV(52, 20, 4, 12);
	glm::vec4 rArmLeftUV    = computeUV(48, 20, 4, 12);
	glm::vec4 rArmRightUV   = computeUV(40, 20, 4, 12);
	glm::vec4 rArmTopUV     = computeUV(44, 16, 4, 4 );
	glm::vec4 rArmBottomUV  = computeUV(48, 16, 4, 4 );

	// Jambe gauche (Left leg)
	glm::vec4 lLegFrontUV   = computeUV(20, 52, 4, 12);
	glm::vec4 lLegBackUV    = computeUV(28, 52, 4, 12);
	glm::vec4 lLegLeftUV    = computeUV(24, 52, 4, 12);
	glm::vec4 lLegRightUV   = computeUV(16, 52, 4, 12);
	glm::vec4 lLegTopUV     = computeUV(20, 48, 4, 4 );
	glm::vec4 lLegBottomUV  = computeUV(24, 48, 4, 4 );

	// Jambe droite (Right leg)
	glm::vec4 rLegFrontUV   = computeUV(4 , 20, 4, 12);
	glm::vec4 rLegBackUV    = computeUV(12, 20, 4, 12);
	glm::vec4 rLegLeftUV    = computeUV(8 , 20, 4, 12);
	glm::vec4 rLegRightUV   = computeUV(0 , 20, 4, 12);
	glm::vec4 rLegTopUV     = computeUV(4 , 16, 4, 4 );
	glm::vec4 rLegBottomUV  = computeUV(8 , 16, 4, 4 );

	// Lambda pour rendre un prisme rectangulaire
	auto renderPrism = [&]( const glm::vec3& center, const glm::vec3& size,
							const glm::vec4& frontUV, const glm::vec4& backUV,
							const glm::vec4& rightUV, const glm::vec4& leftUV,
							const glm::vec4& topUV, const glm::vec4& bottomUV)
	{
		float halfW = size.x / 2.0f;
		float halfH = size.y / 2.0f;
		float halfD = size.z / 2.0f;
		glm::vec3 p0 = center + glm::vec3(-halfW, -halfH, halfD);
		glm::vec3 p1 = center + glm::vec3( halfW, -halfH, halfD);
		glm::vec3 p2 = center + glm::vec3( halfW, halfH, halfD);
		glm::vec3 p3 = center + glm::vec3(-halfW, halfH, halfD);
		glm::vec3 p4 = center + glm::vec3(-halfW, -halfH, -halfD);
		glm::vec3 p5 = center + glm::vec3( halfW, -halfH, -halfD);
		glm::vec3 p6 = center + glm::vec3( halfW, halfH, -halfD);
		glm::vec3 p7 = center + glm::vec3(-halfW, halfH, -halfD);

		const int numFaces = 6;
		const int verticesPerFace = 6;
		const int floatsPerVertex = 5;
		float vertices[numFaces * verticesPerFace * floatsPerVertex];
		int offset = 0;
		auto setFace = [&]( const glm::vec3& a, const glm::vec3& b,
							const glm::vec3& c, const glm::vec3& d,
							const glm::vec4& uv) {
			// Triangle 1: a, b, c
			vertices[offset++] = a.x; vertices[offset++] = a.y; vertices[offset++] = a.z;
			vertices[offset++] = uv.x; vertices[offset++] = uv.y;
			vertices[offset++] = b.x; vertices[offset++] = b.y; vertices[offset++] = b.z;
			vertices[offset++] = uv.z; vertices[offset++] = uv.y;
			vertices[offset++] = c.x; vertices[offset++] = c.y; vertices[offset++] = c.z;
			vertices[offset++] = uv.z; vertices[offset++] = uv.w;
			// Triangle 2: c, d, a
			vertices[offset++] = c.x; vertices[offset++] = c.y; vertices[offset++] = c.z;
			vertices[offset++] = uv.z; vertices[offset++] = uv.w;
			vertices[offset++] = d.x; vertices[offset++] = d.y; vertices[offset++] = d.z;
			vertices[offset++] = uv.x; vertices[offset++] = uv.w;
			vertices[offset++] = a.x; vertices[offset++] = a.y; vertices[offset++] = a.z;
			vertices[offset++] = uv.x; vertices[offset++] = uv.y;
		};

		// Front face: p3, p2, p1, p0
		setFace(p3, p2, p1, p0, frontUV);
		// Back face: p6, p7, p4, p5
		setFace(p6, p7, p4, p5, backUV);
		// Left face: p2, p6, p5, p1
		setFace(p2, p6, p5, p1, leftUV);
		// Right face: p7, p3, p0, p4
		setFace(p7, p3, p0, p4, rightUV);
		// Top face: p7, p6, p2, p3
		setFace(p7, p6, p2, p3, topUV);
		// Bottom face: p4, p5, p1, p0
		setFace(p4, p5, p1, p0, bottomUV);

		GLuint vao, vbo;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	};

	// Rendu de chaque partie du corps :
	// Head
	renderPrism(headCenter, headSize, headFrontUV, headBackUV, headRightUV, headLeftUV, headTopUV, headBottomUV);
	// Torso
	renderPrism(torsoCenter, torsoSize, torsoFrontUV, torsoBackUV, torsoRightUV, torsoLeftUV, torsoTopUV, torsoBottomUV);
	// Left Arm
	renderPrism(leftArmCenter, armSize, lArmFrontUV, lArmBackUV, lArmRightUV, lArmLeftUV, lArmTopUV, lArmBottomUV);
	// Right Arm
	renderPrism(rightArmCenter, armSize, rArmFrontUV, rArmBackUV, rArmRightUV, rArmLeftUV, rArmTopUV, rArmBottomUV);
	// Left Leg
	renderPrism(leftLegCenter, legSize, lLegFrontUV, lLegBackUV, lLegRightUV, lLegLeftUV, lLegTopUV, lLegBottomUV);
	// Right Leg
	renderPrism(rightLegCenter, legSize, rLegFrontUV, rLegBackUV, rLegRightUV, rLegLeftUV, rLegTopUV, rLegBottomUV);
}
} // namespace Game
