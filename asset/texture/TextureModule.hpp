#ifndef TEXTURE_MODULE_HPP
#define TEXTURE_MODULE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <nlohmann/json.hpp>

namespace asset {
namespace texture {

	/// Structure représentant la définition d’une texture telle que chargée depuis le JSON.
	struct TextureDefinition {
		std::string name;         // Nom unique de la texture
		std::string file;         // Chemin vers le fichier image (ex: "asset/texture/grass.png")
		std::string modifiers;    // Chaîne de modificateurs (ex: "^[colorize:#FF000080")
		struct Animation {
			int frametime = 1;
			std::vector<int> frames;
		} animation;
	};

	/// Structure décrivant une région UV dans l’atlas.
	struct TextureRegion {
		glm::vec2 uvMin;
		glm::vec2 uvMax;
	};

	/// Classe en charge de construire l’atlas de textures à partir d’un ensemble de définitions.
	class TextureAtlas {
	public:
		TextureAtlas();
		~TextureAtlas();

		/**
		 * @brief Construit l’atlas à partir d’un vecteur de TextureDefinition.
		 * @param textureDefs Liste des définitions.
		 * @return true si l’atlas est correctement construit, false sinon.
		 */
		bool loadAtlas(const std::vector<TextureDefinition>& textureDefs);

		/**
		 * @brief Renvoie l’ID OpenGL de l’atlas.
		 */
		GLuint getAtlasTextureID() const { return atlasTextureID; }

		/**
		 * @brief Récupère la région UV d’une texture donnée par son nom.
		 * @param name Nom de la texture.
		 * @param region Région UV (en sortie).
		 * @return true si trouvée, false sinon.
		 */
		bool getTextureRegion(const std::string& name, TextureRegion& region) const;
	private:
		GLuint atlasTextureID;
		int atlasWidth;
		int atlasHeight;
		int textureWidth;   // On impose que toutes les textures ont la même taille
		int textureHeight;
		std::unordered_map<std::string, TextureRegion> textureRegions;

		void clearAtlas();
		bool packTextures(const std::vector<std::pair<std::string, std::string>>& textures);
	};

	/// Classe qui charge la configuration JSON et expose l’interface de gestion des textures.
	class TextureManager {
	public:
		TextureManager();
		~TextureManager();

		/**
		 * @brief Charge la configuration des textures depuis un fichier JSON.
		 * @param configFilePath Chemin vers le fichier (ex: "asset/textures.json").
		 * @return true en cas de succès, false sinon.
		 */
		bool loadTextureConfig(const std::string& configFilePath);

		/**
		 * @brief Initialise l’atlas de textures à partir des définitions chargées.
		 * @return true si l’atlas est construit avec succès, false sinon.
		 */
		bool initializeAtlas();

		/**
		 * @brief Récupère la définition d’une texture par son nom.
		 * @param name Nom de la texture.
		 * @param def Structure de sortie contenant la définition.
		 * @return true si trouvée, false sinon.
		 */
		bool getTextureDefinition(const std::string& name, TextureDefinition& def) const;

		/**
		 * @brief Renvoie l’ID OpenGL de l’atlas.
		 */
		GLuint getAtlasTextureID() const;

		/**
		 * @brief Récupère la région UV d’une texture par son nom.
		 * @param name Nom de la texture.
		 * @param region Région UV (en sortie).
		 * @return true si trouvée, false sinon.
		 */
		bool getTextureRegion(const std::string& name, TextureRegion& region) const;
	private:
		std::unordered_map<std::string, TextureDefinition> textureDefinitions;
		TextureAtlas atlas;
	};

} // namespace texture
} // namespace asset

#endif // TEXTURE_MODULE_HPP
