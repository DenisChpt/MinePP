#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <string>
#include <map>
#include <GL/glew.h>

namespace ResourceManager {

	/// Charge une texture depuis le fichier spécifié (format PNG attendu)
	/// et la met en cache pour éviter des rechargements multiples.
	/// Retourne l'ID OpenGL de la texture.
	GLuint loadTexture(const std::string &filePath);

	/// Charge, compile et renvoie un shader de type 'type' (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
	/// à partir du fichier source spécifié.
	GLuint loadShader(GLenum type, const std::string &filePath);

	/// Charge, compile et lie un programme shader à partir des chemins fournis pour
	/// le vertex shader et le fragment shader. Le programme est mis en cache.
	GLuint loadProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

	/// Libère toutes les ressources mises en cache.
	void clear();

} // namespace ResourceManager

#endif // RESOURCE_MANAGER_HPP
