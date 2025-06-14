#pragma once

#include "../Common.hpp"

// Forward declaration
class Assets;

// Shader unique
class Shader {
	uint32_t id;

   public:
	Shader(const std::string& source, uint32_t type);
	Shader(const std::string& name, Assets& assets);
	~Shader();

	[[nodiscard]] bool isValid() const { return id != 0; }
	[[nodiscard]] uint32_t getId() const { return id; }

	Shader(const Shader&) = delete;
	Shader(Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) noexcept = delete;
	Shader& operator=(Shader&&) noexcept = delete;
};

// ShaderProgram
class ShaderProgram {
	uint32_t id = 0;

   public:
	ShaderProgram(const Ref<const Shader>& vertexShader, const Ref<const Shader>& fragmentShader);
	ShaderProgram(const std::string& name, Assets& assets);
	~ShaderProgram();

	void bind() const;

	// Uniform setters
	void setFloat(const std::string& name, float value) const;
	void setInt(const std::string& name, int value) const;
	void setUInt(const std::string& name, uint32_t value) const;
	void setVec2(const std::string& name, const glm::vec2& value) const;
	void setVec3(const std::string& name, const glm::vec3& value) const;
	void setVec4(const std::string& name, const glm::vec4& value) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;
	void setTexture(const std::string& name,
					const Ref<const class Texture>& texture,
					int32_t slot) const;

	[[nodiscard]] bool isValid() const { return id != 0; }
	[[nodiscard]] uint32_t getId() const { return id; }

	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram(ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;
	ShaderProgram(ShaderProgram&&) noexcept = delete;
	ShaderProgram& operator=(ShaderProgram&&) noexcept = delete;
};

// ProceduralShader
class ProceduralShader {
   protected:
	virtual std::string emitVertexShaderSource() const = 0;
	virtual std::string emitFragmentShaderSource() const = 0;

   public:
	Ref<const ShaderProgram> getShader() const;
	virtual ~ProceduralShader() = default;
};