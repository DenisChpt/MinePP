#ifndef DEBUG_OVERLAY_HPP
#define DEBUG_OVERLAY_HPP

#include <string>
#include <glm/glm.hpp>

namespace ui {

	/**
	 * @brief Displays simple debug information, such as FPS and camera position.
	 */
	class DebugOverlay {
	public:
		DebugOverlay();
		~DebugOverlay();

		void setFPS(float fps);
		void setCameraPosition(const glm::vec3 &pos);
		void setDebugInfo(const std::string &info);
		void render(const glm::mat4 &projection);

	private:
		float fps;
		glm::vec3 cameraPos;
		std::string debugInfo;

		void renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
	};

} // namespace ui

#endif // DEBUG_OVERLAY_HPP
