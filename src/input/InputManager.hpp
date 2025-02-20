#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <unordered_map>

namespace input {

class InputManager {
public:
	InputManager();
	~InputManager();

	// Appelés depuis les callbacks GLFW
	void keyCallback(int key, int scancode, int action, int mods);
	void mouseCallback(double xpos, double ypos);
	void scrollCallback(double xoffset, double yoffset);

	// Mise à jour (par exemple réinitialiser les offsets de scroll)
	void update();

	// Interroge si une touche est enfoncée
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
