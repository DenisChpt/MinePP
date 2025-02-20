#include "TextureAnimation.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

bool loadTextureAnimation(const std::string& mcmetaPath, TextureAnimation& animation) {
	std::ifstream file(mcmetaPath);
	if (!file.is_open()) {
		std::cerr << "Erreur : impossible d'ouvrir le fichier mcmeta: " << mcmetaPath << std::endl;
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	json j;
	try {
		j = json::parse(buffer.str());
	} catch (json::parse_error &e) {
		std::cerr << "Erreur de parsing JSON dans le fichier mcmeta: " << e.what() << std::endl;
		return false;
	}
	if (!j.contains("animation") || !j["animation"].is_object()) {
		std::cerr << "Le fichier mcmeta ne contient pas de section 'animation'." << std::endl;
		return false;
	}
	json anim = j["animation"];
	// Le frametime par défaut est 1 si non spécifié.
	animation.frametime = anim.value("frametime", 1);
	if (anim.contains("frames")) {
		// La section frames peut être un tableau d'entiers ou d'objets.
		if (anim["frames"].is_array()) {
			animation.frames.clear();
			for (const auto& frame : anim["frames"]) {
				if (frame.is_number_integer()) {
					animation.frames.push_back(frame.get<int>());
				} else if (frame.is_object() && frame.contains("index")) {
					animation.frames.push_back(frame["index"].get<int>());
				}
			}
		}
	}
	return true;
}
