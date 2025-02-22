#ifndef BLOCK_MODEL_HPP
#define BLOCK_MODEL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

namespace blockmodel {

	/**
	 * @brief Represents a face within a block element, including texture reference and optional UV.
	 */
	struct Face {
		std::string texture;
		std::string cullface;
		std::vector<float> uv;
	};

	/**
	 * @brief Represents a cube-like element within a block model, defined by 'from' and 'to' bounds.
	 */
	struct ParsedBlockElement {
		glm::vec3 from;
		glm::vec3 to;
		std::unordered_map<std::string, Face> faces;
	};

	/**
	 * @brief Contains the entirety of a block model, including parent model and elements.
	 */
	struct ParsedBlockModel {
		std::string parent;
		std::unordered_map<std::string, std::string> textures;
		std::vector<ParsedBlockElement> elements;
	};

} // namespace blockmodel

#endif // BLOCK_MODEL_HPP
