#include "AssetLoader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#include <json.hpp>
using json = nlohmann::json;

namespace asset {

AssetLoader::AssetLoader() {
}

AssetLoader::~AssetLoader() {
}

bool AssetLoader::loadBlockAssets(const std::string &filePath) {
	// Ouvrir le fichier blocks.json dans le répertoire du jeu.
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Erreur : Impossible d'ouvrir le fichier d'assets : " << filePath << std::endl;
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	json j;
	try {
		j = json::parse(buffer.str());
	} catch (json::parse_error &e) {
		std::cerr << "Erreur de parsing JSON : " << e.what() << std::endl;
		return false;
	}

	// Pour chaque bloc (clé du JSON), parsez l'objet associé.
	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string blockId = it.key();
		const json &blockJson = it.value();

		if (!parseBlockAsset(blockId, blockJson)) {
			std::cerr << "Erreur lors du parsing de l'asset pour le bloc " << blockId << std::endl;
			// On peut continuer à parser les autres blocs.
		}
	}
	return true;
}

const std::unordered_map<std::string, BlockAsset>& AssetLoader::getBlockAssets() const {
	return blockAssets;
}

bool AssetLoader::parseBlockAsset(const std::string &blockId, const json &j) {
	BlockAsset asset;
	asset.identifier = blockId;

	// Parser la section "definition"
	if (j.contains("definition") && j["definition"].is_object()) {
		if (!parseDefinition(asset.definition, j["definition"])) {
			std::cerr << "Erreur lors du parsing de la définition pour le bloc " << blockId << std::endl;
			return false;
		}
	} else {
		std::cerr << "Bloc " << blockId << " : section 'definition' manquante." << std::endl;
		return false;
	}

	// Parser la section "properties" (facultatif mais généralement présent)
	if (j.contains("properties") && j["properties"].is_object()) {
		if (!parseProperties(asset.properties, j["properties"])) {
			std::cerr << "Erreur lors du parsing des propriétés pour le bloc " << blockId << std::endl;
			return false;
		}
	}

	// Parser la section "states"
	if (j.contains("states") && j["states"].is_array()) {
		if (!parseStates(asset.states, j["states"])) {
			std::cerr << "Erreur lors du parsing des états pour le bloc " << blockId << std::endl;
			return false;
		}
	} else {
		std::cerr << "Bloc " << blockId << " : section 'states' manquante." << std::endl;
		return false;
	}

	blockAssets[blockId] = asset;
	return true;
}

bool AssetLoader::parseDefinition(BlockDefinition &def, const json &j) {
	if (!j.is_object()) return false;

	// "type" est requis.
	if (j.contains("type") && j["type"].is_string()) {
		def.type = j["type"].get<std::string>();
	} else {
		std::cerr << "La définition ne contient pas le champ 'type'." << std::endl;
		return false;
	}

	// "block_set_type" est optionnel.
	if (j.contains("block_set_type") && j["block_set_type"].is_string()) {
		def.blockSetType = j["block_set_type"].get<std::string>();
	}

	// Pour les autres champs, on les stocke dans def.extra.
	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string key = it.key();
		if (key != "type" && key != "block_set_type") {
			if (it.value().is_string()) {
				def.extra[key] = it.value().get<std::string>();
			} else {
				// Si ce n'est pas une simple chaîne, on dump la valeur.
				def.extra[key] = it.value().dump();
			}
		}
	}
	return true;
}

bool AssetLoader::parseProperties(BlockProperties &props, const json &j) {
	if (!j.is_object()) return false;

	// Chaque clé doit pointer vers un tableau de chaînes.
	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string propName = it.key();
		if (!it.value().is_array()) continue;
		std::vector<std::string> values;
		for (const auto &v : it.value()) {
			if (v.is_string()) {
				values.push_back(v.get<std::string>());
			}
		}
		props.properties[propName] = values;
	}
	return true;
}

bool AssetLoader::parseStates(std::vector<BlockState> &states, const json &j) {
	if (!j.is_array()) return false;
	for (const auto &stateJson : j) {
		BlockState state;
		state.isDefault = false;

		// Le champ "id" est requis.
		if (stateJson.contains("id") && stateJson["id"].is_number_integer()) {
			state.id = stateJson["id"].get<int>();
		} else {
			std::cerr << "Un état est dépourvu du champ 'id'." << std::endl;
			continue;
		}

		// Le champ "default" est optionnel.
		if (stateJson.contains("default") && stateJson["default"].is_boolean()) {
			state.isDefault = stateJson["default"].get<bool>();
		}

		// Le champ "properties" est requis.
		if (stateJson.contains("properties") && stateJson["properties"].is_object()) {
			for (auto it = stateJson["properties"].begin(); it != stateJson["properties"].end(); ++it) {
				std::string key = it.key();
				if (it.value().is_string()) {
					state.properties[key] = it.value().get<std::string>();
				}
			}
		} else {
			std::cerr << "L'état avec id " << state.id << " est dépourvu de 'properties'." << std::endl;
		}
		states.push_back(state);
	}
	return true;
}

} // namespace asset
