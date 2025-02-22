#include "DebugOverlay.hpp"
#include <GL/glut.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>

namespace ui {

DebugOverlay::DebugOverlay() 
	: fps(0.0f),
	  cameraPos(0.0f),
	  debugInfo("")
{
}

DebugOverlay::~DebugOverlay() {
}

void DebugOverlay::setFPS(float fps) {
	this->fps = fps;
}

void DebugOverlay::setCameraPosition(const glm::vec3 &pos) {
	cameraPos = pos;
}

void DebugOverlay::setDebugInfo(const std::string &info) {
	debugInfo = info;
}

void DebugOverlay::render(const glm::mat4 &projection) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(projection));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderText("FPS: " + std::to_string(fps), 10.0f, 10.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
	renderText("Cam Pos: (" + std::to_string(cameraPos.x) + ", " + 
			   std::to_string(cameraPos.y) + ", " + std::to_string(cameraPos.z) + ")",
			   10.0f, 30.0f, 1.0f, glm::vec3(0.0f, 1.0f, 1.0f));
	renderText(debugInfo, 10.0f, 50.0f, 1.0f, glm::vec3(1.0f, 0.5f, 0.0f));

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void DebugOverlay::renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color) {
	glColor3f(color.r, color.g, color.b);
	glRasterPos2f(x, y);
	for (char c : text) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
	}
}

} // namespace ui
