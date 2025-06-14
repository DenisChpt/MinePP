/**
 * @class Persistence
 * @brief Gère la sauvegarde et le chargement des données de la scène (chunks, caméra).
 *
 * @details La classe Persistence lit et écrit un fichier binaire qui contient la position de la
 * caméra et les données de chaque chunk. Elle offre des méthodes pour committer (sauvegarder) un
 * chunk ou la caméra, et pour récupérer un chunk à partir de son identifiant de position.
 *
 * @param path Chemin du fichier de sauvegarde.
 */

#pragma once

#include "../Common.hpp"
#include "../Scene/Camera.hpp"
#include "../Utils/Utils.hpp"
#include "../World/Chunk.hpp"

class Persistence {
	std::string path;
	Camera camera;
	std::unordered_map<glm::ivec2, Ref<Chunk>, Util::HashVec2> chunks;

   public:
	explicit Persistence(std::string path);
	~Persistence();

	void commitChunk(const Ref<Chunk>& chunk);
	[[nodiscard]] Ref<Chunk> getChunk(glm::ivec2 position) const;

	void commitCamera(const Camera& newCamera);
	[[nodiscard]] const Camera& getCamera() const;
};