/**
 * @class ProceduralShader
 * @brief Classe abstraite permettant de générer dynamiquement des shaders procéduraux.
 *
 * @details Les classes dérivées doivent implémenter les méthodes emitVertexShaderSource() et
 *          emitFragmentShaderSource() pour fournir le code source GLSL correspondant.
 *
 * @note Une conversion en ShaderProgram est proposée via l'opérateur de conversion.
 */


#pragma once

#include "ShaderProgram.hpp"

class ProceduralShader
{
protected:
	[[nodiscard]] virtual std::string emitVertexShaderSource() const = 0;
	[[nodiscard]] virtual std::string emitFragmentShaderSource() const = 0;

public:
	explicit operator Ref<const ShaderProgram>() const { return getShader(); };
	[[nodiscard]] Ref<const ShaderProgram> getShader() const;

	virtual ~ProceduralShader() = default;
};
