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
		std::cerr << "Error: Cannot open model file: " << filePath << std::endl;
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	nlohmann::json j;
	try {
		j = nlohmann::json::parse(buffer.str());
	} catch (nlohmann::json::parse_error &e) {
		std::cerr << "Error parsing JSON in model: " << e.what() << std::endl;
		return false;
	}
	return parseModel(j, outModel);
}

bool BlockModelParser::parseModel(const nlohmann::json& j, ParsedBlockModel& outModel) {
	if (!j.is_object()) {
		std::cerr << "JSON model is not an object." << std::endl;
		return false;
	}
	if (j.contains("parent") && j["parent"].is_string()) {
		outModel.parent = j["parent"].get<std::string>();
	}
	if (j.contains("textures") && j["textures"].is_object()) {
		for (auto it = j["textures"].begin(); it != j["textures"].end(); ++it) {
			if (it.value().is_string()) {
				outModel.textures[it.key()] = it.value().get<std::string>();
			}
		}
	}
	if (j.contains("elements") && j["elements"].is_array()) {
		for (const auto &elemJson : j["elements"]) {
			ParsedBlockElement element;
			if (parseElement(elemJson, element)) {
				outModel.elements.push_back(element);
			} else {
				std::cerr << "Error parsing a model element." << std::endl;
			}
		}
	}
	return true;
}

bool BlockModelParser::parseElement(const nlohmann::json& j, ParsedBlockElement& outElement) {
	if (!j.is_object()) {
		std::cerr << "Model element is not a JSON object." << std::endl;
		return false;
	}
	if (j.contains("from") && j["from"].is_array() && j["from"].size() == 3) {
		outElement.from = glm::vec3(j["from"][0].get<float>(),
									j["from"][1].get<float>(),
									j["from"][2].get<float>());
	} else {
		std::cerr << "Element lacks a valid 'from' vector." << std::endl;
		return false;
	}
	if (j.contains("to") && j["to"].is_array() && j["to"].size() == 3) {
		outElement.to = glm::vec3(j["to"][0].get<float>(),
								  j["to"][1].get<float>(),
								  j["to"][2].get<float>());
	} else {
		std::cerr << "Element lacks a valid 'to' vector." << std::endl;
		return false;
	}
	if (j.contains("faces") && j["faces"].is_object()) {
		for (auto it = j["faces"].begin(); it != j["faces"].end(); ++it) {
			Face face;
			if (parseFace(it.value(), face)) {
				outElement.faces[it.key()] = face;
			} else {
				std::cerr << "Error parsing a face in the element." << std::endl;
			}
		}
	}
	return true;
}

bool BlockModelParser::parseFace(const nlohmann::json& j, Face& outFace) {
	if (!j.is_object()) {
		std::cerr << "Face is not a JSON object." << std::endl;
		return false;
	}
	if (j.contains("texture") && j["texture"].is_string()) {
		outFace.texture = j["texture"].get<std::string>();
	} else {
		std::cerr << "Face lacks 'texture' field." << std::endl;
		return false;
	}
	if (j.contains("cullface") && j["cullface"].is_string()) {
		outFace.cullface = j["cullface"].get<std::string>();
	}
	if (j.contains("uv") && j["uv"].is_array() && j["uv"].size() == 4) {
		outFace.uv.clear();
		for (const auto& value : j["uv"]) {
			outFace.uv.push_back(value.get<float>());
		}
	}
	return true;
}

} // namespace blockmodel
