#ifndef ASSET_LOADER_HPP
#define ASSET_LOADER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <json.hpp>

namespace asset {

	/**
	 * @brief Holds definition details for a block (e.g. block type, additional properties).
	 */
	struct BlockDefinition {
		std::string type;
		std::string blockSetType;
		std::unordered_map<std::string, std::string> extra;
	};

	/**
	 * @brief Holds property definitions for a block, listing possible values for each property.
	 */
	struct BlockProperties {
		std::unordered_map<std::string, std::vector<std::string>> properties;
	};

	/**
	 * @brief Represents a possible state of a block, including property values and an ID.
	 */
	struct BlockState {
		int id;
		bool isDefault;
		std::unordered_map<std::string, std::string> properties;
	};

	/**
	 * @brief Encapsulates all data for a given block, including definition, properties, and states.
	 */
	struct BlockAsset {
		std::string identifier;
		BlockDefinition definition;
		BlockProperties properties;
		std::vector<BlockState> states;
	};

	/**
	 * @brief Responsible for loading and parsing block assets (JSON data).
	 */
	class AssetLoader {
	public:
		AssetLoader();
		~AssetLoader();

		/**
		 * @brief Loads block asset data from a specified JSON file.
		 * @param filePath Path to the blocks.json file.
		 * @return True if successfully loaded and parsed, otherwise false.
		 */
		bool loadBlockAssets(const std::string &filePath);

		/**
		 * @brief Provides access to all loaded block assets.
		 * @return A const reference to the map of block identifier to BlockAsset.
		 */
		const std::unordered_map<std::string, BlockAsset>& getBlockAssets() const;

	private:
		std::unordered_map<std::string, BlockAsset> blockAssets;

		bool parseBlockAsset(const std::string &blockId, const nlohmann::json &j);
		bool parseDefinition(BlockDefinition &def, const nlohmann::json &j);
		bool parseProperties(BlockProperties &props, const nlohmann::json &j);
		bool parseStates(std::vector<BlockState> &states, const nlohmann::json &j);
	};

} // namespace asset

#endif // ASSET_LOADER_HPP
