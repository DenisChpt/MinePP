#ifndef ASSET_LOADER_HPP
#define ASSET_LOADER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "../external/nlohmann/json.hpp"

namespace asset {

	// Structure représentant la section "definition" d'un bloc.
	struct BlockDefinition {
		std::string type;         // Exemple : "minecraft:button"
		std::string blockSetType; // Par exemple, "acacia", si présent.
		// Dictionnaire pour stocker d'autres propriétés (ex. "ticks_to_stay_pressed", etc.)
		std::unordered_map<std::string, std::string> extra;
	};

	// Structure représentant la section "properties" qui liste les valeurs possibles.
	struct BlockProperties {
		// Pour chaque propriété, la liste de ses valeurs possibles.
		std::unordered_map<std::string, std::vector<std::string>> properties;
	};

	// Structure représentant un état de bloc.
	struct BlockState {
		int id;
		bool isDefault; // indique si cet état est par défaut.
		// Map des propriétés pour cet état (par exemple, {"face": "floor", "facing": "north", "powered": "true"})
		std::unordered_map<std::string, std::string> properties;
	};

	// Structure globale regroupant la définition d'un bloc.
	struct BlockAsset {
		std::string identifier; // Par exemple "minecraft:acacia_button"
		BlockDefinition definition;
		BlockProperties properties;
		std::vector<BlockState> states;
	};

	// Classe chargée de lire et parser le fichier blocks.json.
	class AssetLoader {
	public:
		AssetLoader();
		~AssetLoader();

		// Charge et parse le fichier de blocs depuis le chemin indiqué.
		// Le fichier blocks.json doit être placé dans le répertoire du jeu (par exemple "./blocks.json")
		bool loadBlockAssets(const std::string &filePath);

		// Accès aux assets chargés.
		const std::unordered_map<std::string, BlockAsset>& getBlockAssets() const;

	private:
		std::unordered_map<std::string, BlockAsset> blockAssets;

		// Fonctions d'aide pour parser les différentes sections du JSON.
		bool parseBlockAsset(const std::string &blockId, const nlohmann::json &j);
		bool parseDefinition(BlockDefinition &def, const nlohmann::json &j);
		bool parseProperties(BlockProperties &props, const nlohmann::json &j);
		bool parseStates(std::vector<BlockState> &states, const nlohmann::json &j);
	};

} // namespace asset

#endif // ASSET_LOADER_HPP
