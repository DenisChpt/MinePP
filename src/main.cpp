#include "Application.hpp"
#include <iostream>

/**
 * @brief Entry point of the MinePP application.
 */
int main() {
	Application app(800, 600, "MinePP");
	if (!app.initialize()) {
		std::cerr << "Failed to initialize the application." << std::endl;
		return -1;
	}
	app.run();
	return 0;
}
