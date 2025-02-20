#ifndef BLOCK_MODEL_PARSER_HPP
#define BLOCK_MODEL_PARSER_HPP

#include "BlockModel.hpp"
#include <json.hpp>
#include <string>

namespace blockmodel {

/**
 * Classe permettant de parser un fichier JSON définissant un modèle de bloc.
 */
class BlockModelParser {
public:
	BlockModelParser();
	~BlockModelParser();

	/**
	 * Parse un modèle de bloc depuis un fichier JSON.
	 * @param filePath : chemin du fichier modèle.
	 * @param outModel : (out) modèle parsé.
	 * @return true en cas de succès.
	 */
	bool parseModelFromFile(const std::string& filePath, ParsedBlockModel& outModel);

	/**
	 * Parse un modèle de bloc à partir d'un objet JSON.
	 * @param j : objet JSON représentant le modèle.
	 * @param outModel : (out) modèle parsé.
	 * @return true en cas de succès.
	 */
	bool parseModel(const nlohmann::json& j, ParsedBlockModel& outModel);

private:
	bool parseElement(const nlohmann::json& j, ParsedBlockElement& outElement);
	bool parseFace(const nlohmann::json& j, Face& outFace);
};

} // namespace blockmodel

#endif // BLOCK_MODEL_PARSER_HPP
