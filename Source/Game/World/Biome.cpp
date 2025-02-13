#include "Biome.hpp"

namespace Game {

Biome::Biome(const std::string& name)
	: m_Name(name) {
}

Biome::~Biome() {
}

const std::string& Biome::getName() const {
	return m_Name;
}

} // namespace Game
