#include "Application.hpp"
#include <iostream>

int main() {
	Application app(800, 600, "MinePP");
	if (!app.initialize()) {
		std::cerr << "Échec de l'initialisation de l'application" << std::endl;
		return -1;
	}
	app.run();
	return 0;
}
