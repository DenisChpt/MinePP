#include "Shaders.hpp"
#include "../Core/Assets.hpp"
#include "../Utils/Utils.hpp"
#include "Textures.hpp"

// Shader implementation
Shader::Shader(const std::string &source, uint32_t type) {
    TRACE_FUNCTION();
    assert(type != 0 && "Couldn't identify the shader type");

    id = glCreateShader(type);
    const char *vertexShaderSourceStr = source.c_str();
    glShaderSource(id, 1, &vertexShaderSourceStr, nullptr);
    glCompileShader(id);

    int32_t success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[2048];
        glGetShaderInfoLog(id, sizeof(infoLog) / sizeof(infoLog[0]), nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;

        id = 0;
    }
}

Shader::Shader(const std::string &name, Assets& assets)
    : Shader(*assets.loadText(name),
             name.ends_with(".vert") ? GL_VERTEX_SHADER : (name.ends_with(".frag") ? GL_FRAGMENT_SHADER : 0)) {}

Shader::~Shader() {
    if (isValid()) {
        glDeleteShader(id);
    }
}

// ShaderProgram implementation
ShaderProgram::ShaderProgram(const Ref<const Shader> &vertexShader, const Ref<const Shader> &fragmentShader) {
    TRACE_FUNCTION();
    assert(vertexShader != nullptr && vertexShader->isValid());
    assert(fragmentShader != nullptr && fragmentShader->isValid());

    id = glCreateProgram();
    glAttachShader(id, vertexShader->getId());
    glAttachShader(id, fragmentShader->getId());
    glLinkProgram(id);

    // Check for errors
    int32_t success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[2048];
        glGetProgramInfoLog(id, sizeof(infoLog) / sizeof(infoLog[0]), nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        id = 0;
    }
}

ShaderProgram::ShaderProgram(const std::string &name, Assets& assets) {
    TRACE_FUNCTION();
    Ref<const Shader> vertex = assets.loadShader(name + ".vert");
    Ref<const Shader> fragment = assets.loadShader(name + ".frag");

    if (!vertex || !fragment) {
        std::cerr << "Failed to load shaders: " << name << std::endl;
        id = 0;
        return;
    }

    // Duplicate the linking code instead of using placement new
    id = glCreateProgram();
    glAttachShader(id, vertex->getId());
    glAttachShader(id, fragment->getId());
    glLinkProgram(id);

    // Check for errors
    int32_t success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[2048];
        glGetProgramInfoLog(id, sizeof(infoLog) / sizeof(infoLog[0]), nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        id = 0;
    }
}

ShaderProgram::~ShaderProgram() {
    if (isValid()) {
        glDeleteProgram(id);
    }
}

void ShaderProgram::bind() const {
    TRACE_FUNCTION();
    glUseProgram(id);
}

// Uniform setters
void ShaderProgram::setFloat(const std::string &name, float value) const {
    TRACE_FUNCTION();
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderProgram::setInt(const std::string &name, int value) const {
    TRACE_FUNCTION();
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderProgram::setUInt(const std::string &name, uint32_t value) const {
    TRACE_FUNCTION();
    glUniform1ui(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderProgram::setVec2(const std::string &name, const glm::vec2 &value) const {
    TRACE_FUNCTION();
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &value) const {
    TRACE_FUNCTION();
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void ShaderProgram::setVec4(const std::string &name, const glm::vec4 &value) const {
    TRACE_FUNCTION();
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &mat) const {
    TRACE_FUNCTION();
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setTexture(const std::string &name, const Ref<const Texture> &texture, int32_t slot) const {
    TRACE_FUNCTION();
    setInt(name, slot);
    texture->bindToSlot(slot);
}

// ProceduralShader implementation
Ref<const ShaderProgram> ProceduralShader::getShader() const {
    TRACE_FUNCTION();
    static std::map<const ProceduralShader *, WeakRef<const ShaderProgram>> cache;

    if (cache.contains(this)) {
        if (auto cachedShader = cache[this].lock()) {
            return cachedShader;
        }
    }

    Ref<const Shader> vertexShader = std::make_shared<const Shader>(emitVertexShaderSource(), GL_VERTEX_SHADER);
    Ref<const Shader> fragmentShader = std::make_shared<const Shader>(emitFragmentShaderSource(), GL_FRAGMENT_SHADER);

    Ref<const ShaderProgram> shader = std::make_shared<const ShaderProgram>(vertexShader, fragmentShader);
    cache[this] = shader;

    return shader;
}