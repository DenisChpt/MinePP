#include "BlockModelParser.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

namespace blockmodel {

BlockModelParser::BlockModelParser() {
}

BlockModelParser::~BlockModelParser() {
}

bool BlockModelParser::parseModelFromFile(const std::string& filePath, ParsedBlockModel& outModel) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Erreur : impossible d'ouvrir le fichier modèle : " << filePath << std::endl;
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	nlohmann::json j;
	try {
		j = nlohmann::json::parse(buffer.str());
	} catch (nlohmann::json::parse_error &e) {
		std::cerr << "Erreur de parsing JSON dans le modèle : " << e.what() << std::endl;
		return false;
	}
	return parseModel(j, outModel);
}

bool BlockModelParser::parseModel(const nlohmann::json& j, ParsedBlockModel& outModel) {
	if (!j.is_object()) {
		std::cerr << "Le modèle JSON n'est pas un objet." << std::endl;
		return false;
	}
	// Récupère le champ "parent" s'il existe.
	if (j.contains("parent") && j["parent"].is_string()) {
		outModel.parent = j["parent"].get<std::string>();
	}
	// Récupère la section "textures".
	if (j.contains("textures") && j["textures"].is_object()) {
		for (auto it = j["textures"].begin(); it != j["textures"].end(); ++it) {
			if (it.value().is_string()) {
				outModel.textures[it.key()] = it.value().get<std::string>();
			}
		}
	}
	// Récupère les éléments (éléments du modèle).
	if (j.contains("elements") && j["elements"].is_array()) {
		for (const auto &elemJson : j["elements"]) {
			ParsedBlockElement element;
			if (parseElement(elemJson, element)) {
				outModel.elements.push_back(element);
			} else {
				std::cerr << "Erreur lors du parsing d'un élément du modèle." << std::endl;
			}
		}
	}
	return true;
}

bool BlockModelParser::parseElement(const nlohmann::json& j, ParsedBlockElement& outElement) {
	if (!j.is_object()) {
		std::cerr << "L'élément du modèle n'est pas un objet JSON." << std::endl;
		return false;
	}
	// Parse du vecteur "from" (tableau de 3 nombres).
	if (j.contains("from") && j["from"].is_array() && j["from"].size() == 3) {
		outElement.from = glm::vec3(j["from"][0].get<float>(), j["from"][1].get<float>(), j["from"][2].get<float>());
	} else {
		std::cerr << "L'élément du modèle manque de 'from' valide." << std::endl;
		return false;
	}
	// Parse du vecteur "to".
	if (j.contains("to") && j["to"].is_array() && j["to"].size() == 3) {
		outElement.to = glm::vec3(j["to"][0].get<float>(), j["to"][1].get<float>(), j["to"][2].get<float>());
	} else {
		std::cerr << "L'élément du modèle manque de 'to' valide." << std::endl;
		return false;
	}
	// Parse des faces.
	if (j.contains("faces") && j["faces"].is_object()) {
		for (auto it = j["faces"].begin(); it != j["faces"].end(); ++it) {
			Face face;
			if (parseFace(it.value(), face)) {
				outElement.faces[it.key()] = face;
			} else {
				std::cerr << "Erreur lors du parsing d'une face dans l'élément." << std::endl;
			}
		}
	}
	return true;
}

bool BlockModelParser::parseFace(const nlohmann::json& j, Face& outFace) {
	if (!j.is_object()) {
		std::cerr << "La face n'est pas un objet JSON." << std::endl;
		return false;
	}
	// Le champ "texture" est obligatoire.
	if (j.contains("texture") && j["texture"].is_string()) {
		outFace.texture = j["texture"].get<std::string>();
	} else {
		std::cerr << "La face manque du champ 'texture'." << std::endl;
		return false;
	}
	// Optionnel : "cullface"
	if (j.contains("cullface") && j["cullface"].is_string()) {
		outFace.cullface = j["cullface"].get<std::string>();
	}
	// Optionnel : "uv"
	if (j.contains("uv") && j["uv"].is_array() && j["uv"].size() == 4) {
		outFace.uv.clear();
		for (const auto& value : j["uv"]) {
			outFace.uv.push_back(value.get<float>());
		}
	}
	return true;
}

} // namespace blockmodel
