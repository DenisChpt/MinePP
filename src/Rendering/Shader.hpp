/**
 * @class Shader
 * @brief Représente un shader OpenGL (vertex ou fragment) et gère sa compilation.
 *
 * @details Le constructeur compile le code source GLSL et vérifie le statut de compilation.
 *          En cas d'erreur, un message détaillé est affiché et l'identifiant est invalidé.
 *
 * @param source Code source GLSL.
 * @param type Type de shader (GL_VERTEX_SHADER ou GL_FRAGMENT_SHADER).
 */

#pragma once

#include "../Common.hpp"

class AssetManager;

class Shader
{
	uint32_t id;

public:
	Shader(const std::string &source, uint32_t type);
	Shader(const std::string &name, AssetManager& assetManager);
	~Shader();

	[[nodiscard]] bool isValid() const { return id != 0; };
	[[nodiscard]] uint32_t getId() const { return id; };

	Shader(const Shader &) = delete;
	Shader(Shader &) = delete;
	Shader(Shader &&) noexcept = delete;
	Shader &operator=(Shader &) = delete;
	Shader &operator=(Shader &&) noexcept = delete;
};
