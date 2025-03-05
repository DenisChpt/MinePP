#pragma once

#include "../Scene/Camera.hpp"
#include "../Util/Util.hpp"
#include "../World/Chunk.hpp"
#include "../MinePP.hpp"

class Persistence
{
	std::string path;
	Camera camera;
	std::unordered_map<glm::ivec2, Ref<Chunk>, Util::HashVec2> chunks;

public:
	explicit Persistence(std::string path);
	~Persistence();

	void commitChunk(const Ref<Chunk> &chunk);
	[[nodiscard]] Ref<Chunk> getChunk(glm::ivec2 position) const;

	void commitCamera(const Camera &newCamera);
	[[nodiscard]] const Camera &getCamera() const;
};