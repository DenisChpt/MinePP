#ifndef CHAT_WINDOW_HPP
#define CHAT_WINDOW_HPP

#include <string>
#include <deque>
#include <glm/glm.hpp>

namespace ui {

class ChatWindow {
public:
	ChatWindow();
	~ChatWindow();

	// Ajoute un message au chat
	void addMessage(const std::string &message);

	// Rend le chat
	void render(const glm::mat4 &projection);

	// Traite l'entrée clavier lorsque le chat est actif
	void processInput(int key, int action);

	// Active/désactive le chat
	void toggle();

	// Indique si le chat est actif
	bool isActive() const;

private:
	bool active;
	std::deque<std::string> messages;
	std::string currentInput;
	float backgroundOpacity;
	size_t maxMessages;

	// Fonction d'aide pour le rendu de texte (ici avec GLUT pour la simplicité)
	void renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
};

} // namespace ui

#endif // CHAT_WINDOW_HPP
