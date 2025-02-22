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
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Error: Cannot open asset file: " << filePath << std::endl;
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	json j;
	try {
		j = json::parse(buffer.str());
	} catch (json::parse_error &e) {
		std::cerr << "JSON parsing error: " << e.what() << std::endl;
		return false;
	}

	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string blockId = it.key();
		const json &blockJson = it.value();
		if (!parseBlockAsset(blockId, blockJson)) {
			std::cerr << "Error while parsing block asset for: " << blockId << std::endl;
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

	if (j.contains("definition") && j["definition"].is_object()) {
		if (!parseDefinition(asset.definition, j["definition"])) {
			std::cerr << "Error while parsing definition for block: " << blockId << std::endl;
			return false;
		}
	} else {
		std::cerr << "Block " << blockId << " is missing a 'definition' section." << std::endl;
		return false;
	}

	if (j.contains("properties") && j["properties"].is_object()) {
		if (!parseProperties(asset.properties, j["properties"])) {
			std::cerr << "Error while parsing properties for block: " << blockId << std::endl;
			return false;
		}
	}

	if (j.contains("states") && j["states"].is_array()) {
		if (!parseStates(asset.states, j["states"])) {
			std::cerr << "Error while parsing states for block: " << blockId << std::endl;
			return false;
		}
	} else {
		std::cerr << "Block " << blockId << " is missing a 'states' section." << std::endl;
		return false;
	}

	blockAssets[blockId] = asset;
	return true;
}

bool AssetLoader::parseDefinition(BlockDefinition &def, const json &j) {
	if (!j.is_object()) {
		return false;
	}

	if (j.contains("type") && j["type"].is_string()) {
		def.type = j["type"].get<std::string>();
	} else {
		std::cerr << "Definition lacks 'type' field." << std::endl;
		return false;
	}

	if (j.contains("block_set_type") && j["block_set_type"].is_string()) {
		def.blockSetType = j["block_set_type"].get<std::string>();
	}

	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string key = it.key();
		if (key != "type" && key != "block_set_type") {
			if (it.value().is_string()) {
				def.extra[key] = it.value().get<std::string>();
			} else {
				def.extra[key] = it.value().dump();
			}
		}
	}
	return true;
}

bool AssetLoader::parseProperties(BlockProperties &props, const json &j) {
	if (!j.is_object()) {
		return false;
	}

	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string propName = it.key();
		if (!it.value().is_array()) {
			continue;
		}
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
	if (!j.is_array()) {
		return false;
	}
	for (const auto &stateJson : j) {
		BlockState state;
		state.isDefault = false;

		if (stateJson.contains("id") && stateJson["id"].is_number_integer()) {
			state.id = stateJson["id"].get<int>();
		} else {
			std::cerr << "A block state is missing 'id'." << std::endl;
			continue;
		}

		if (stateJson.contains("default") && stateJson["default"].is_boolean()) {
			state.isDefault = stateJson["default"].get<bool>();
		}

		if (stateJson.contains("properties") && stateJson["properties"].is_object()) {
			for (auto it = stateJson["properties"].begin(); it != stateJson["properties"].end(); ++it) {
				std::string key = it.key();
				if (it.value().is_string()) {
					state.properties[key] = it.value().get<std::string>();
				}
			}
		} else {
			std::cerr << "Block state with id " << state.id << " lacks 'properties'." << std::endl;
		}
		states.push_back(state);
	}
	return true;
}

} // namespace asset
