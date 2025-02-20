#include "ChatWindow.hpp"
#include <iostream>
#include <GL/glut.h>
#include <glm/gtc/type_ptr.hpp>

namespace ui {

ChatWindow::ChatWindow() : active(false), backgroundOpacity(0.5f), maxMessages(10) { }

ChatWindow::~ChatWindow() { }

void ChatWindow::addMessage(const std::string &message) {
	messages.push_back(message);
	if (messages.size() > maxMessages)
		messages.pop_front();
}

void ChatWindow::toggle() {
	active = !active;
}

bool ChatWindow::isActive() const {
	return active;
}

void ChatWindow::processInput(int key, int action) {
	// Très simplifié : 257 = Enter, 259 = Backspace (codes GLFW)
	if (action == 1) { // touche pressée
		if (key == 257) { // Enter
			if (!currentInput.empty()) {
				addMessage(currentInput);
				currentInput.clear();
			}
		} else if (key == 259) { // Backspace
			if (!currentInput.empty())
				currentInput.pop_back();
		} else {
			// Pour simplifier, on ajoute le caractère correspondant au code (pour ASCII)
			char c = static_cast<char>(key);
			currentInput.push_back(c);
		}
	}
}

void ChatWindow::render(const glm::mat4 &projection) {
	// Rendu d'un fond semi-transparent
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(projection));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, backgroundOpacity);
	glBegin(GL_QUADS);
	glVertex2f(10.0f, 10.0f);
	glVertex2f(310.0f, 10.0f);
	glVertex2f(310.0f, 210.0f);
	glVertex2f(10.0f, 210.0f);
	glEnd();

	// Rendu des messages
	float y = 30.0f;
	for (const auto &msg : messages) {
		renderText(msg, 20.0f, y, 1.0f, glm::vec3(1.0f));
		y += 20.0f;
	}
	// Affiche la saisie en cours si le chat est actif
	if (active) {
		renderText("> " + currentInput, 20.0f, y, 1.0f, glm::vec3(0.8f, 0.8f, 0.8f));
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void ChatWindow::renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color) {
	glColor3f(color.r, color.g, color.b);
	glRasterPos2f(x, y);
	for (char c : text) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
	}
}

} // namespace ui
