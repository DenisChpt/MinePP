#ifndef DEBUG_OVERLAY_HPP
#define DEBUG_OVERLAY_HPP

#include <string>
#include <glm/glm.hpp>

namespace ui {

class DebugOverlay {
public:
	DebugOverlay();
	~DebugOverlay();

	// Met à jour la valeur FPS
	void setFPS(float fps);

	// Met à jour la position de la caméra
	void setCameraPosition(const glm::vec3 &pos);

	// Met à jour une info additionnelle (ex. infos sur le chunk)
	void setDebugInfo(const std::string &info);

	// Rendu de l’overlay
	void render(const glm::mat4 &projection);

private:
	float fps;
	glm::vec3 cameraPos;
	std::string debugInfo;

	void renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
};

} // namespace ui

#endif // DEBUG_OVERLAY_HPP
