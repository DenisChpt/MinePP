#ifndef BLOCK_MODEL_PARSER_HPP
#define BLOCK_MODEL_PARSER_HPP

#include "BlockModel.hpp"
#include <json.hpp>
#include <string>

namespace blockmodel {

	/**
	 * @brief Parses JSON files representing block models.
	 */
	class BlockModelParser {
	public:
		BlockModelParser();
		~BlockModelParser();

		/**
		 * @brief Parses a block model from a file on disk.
		 * @param filePath Path to the JSON model file.
		 * @param outModel Output block model data structure.
		 * @return True on success, false on failure.
		 */
		bool parseModelFromFile(const std::string& filePath, ParsedBlockModel& outModel);

		/**
		 * @brief Parses a block model from a JSON object in memory.
		 * @param j The JSON object containing the model data.
		 * @param outModel Output block model data structure.
		 * @return True on success, false on failure.
		 */
		bool parseModel(const nlohmann::json& j, ParsedBlockModel& outModel);

	private:
		bool parseElement(const nlohmann::json& j, ParsedBlockElement& outElement);
		bool parseFace(const nlohmann::json& j, Face& outFace);
	};

} // namespace blockmodel

#endif // BLOCK_MODEL_PARSER_HPP
