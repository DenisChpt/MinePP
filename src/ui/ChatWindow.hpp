#ifndef CHAT_WINDOW_HPP
#define CHAT_WINDOW_HPP

#include <string>
#include <deque>
#include <glm/glm.hpp>

namespace ui {

	/**
	 * @brief A simplistic in-game chat window that handles text input and message display.
	 */
	class ChatWindow {
	public:
		ChatWindow();
		~ChatWindow();

		/**
		 * @brief Adds a message to the internal queue.
		 */
		void addMessage(const std::string &message);

		/**
		 * @brief Renders the chat window overlay.
		 */
		void render(const glm::mat4 &projection);

		/**
		 * @brief Handles keyboard input events when the chat is active.
		 */
		void processInput(int key, int action);

		/**
		 * @brief Toggles the chat window's active state.
		 */
		void toggle();

		/**
		 * @brief Checks if the chat window is currently active.
		 */
		bool isActive() const;

	private:
		bool active;
		std::deque<std::string> messages;
		std::string currentInput;
		float backgroundOpacity;
		size_t maxMessages;

		void renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
	};

} // namespace ui

#endif // CHAT_WINDOW_HPP
