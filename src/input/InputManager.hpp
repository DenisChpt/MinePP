#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <unordered_map>

namespace input {

	/**
	 * @brief Manages input states, including keyboard, mouse, and scroll.
	 */
	class InputManager {
	public:
		InputManager();
		~InputManager();

		/**
		 * @brief Called when a key event occurs.
		 */
		void keyCallback(int key, int scancode, int action, int mods);

		/**
		 * @brief Called when a mouse move event occurs.
		 */
		void mouseCallback(double xpos, double ypos);

		/**
		 * @brief Called when a scroll event occurs.
		 */
		void scrollCallback(double xoffset, double yoffset);

		/**
		 * @brief Updates any transient input state if needed.
		 */
		void update();

		/**
		 * @brief Checks if a key is currently held down.
		 */
		bool isKeyPressed(int key) const;

		double getMouseX() const;
		double getMouseY() const;

	private:
		std::unordered_map<int, bool> keys;
		double mouseX, mouseY;
		double scrollX, scrollY;
	};

} // namespace input

#endif // INPUT_MANAGER_HPP
