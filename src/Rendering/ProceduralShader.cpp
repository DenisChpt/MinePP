#include "ProceduralShader.hpp"

#include "../Performance/Trace.hpp"

Ref<const ShaderProgram> ProceduralShader::getShader() const
{
	TRACE_FUNCTION();
	Ref<const Shader> vertexShader = std::make_shared<const Shader>(emitVertexShaderSource(), GL_VERTEX_SHADER);
	Ref<const Shader> fragmentShader = std::make_shared<const Shader>(emitFragmentShaderSource(), GL_FRAGMENT_SHADER);

	return std::make_shared<const ShaderProgram>(vertexShader, fragmentShader);
}
