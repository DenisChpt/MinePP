#pragma once
#include <string>

namespace Game {

class Biome {
public:
	Biome(const std::string& name);
	~Biome();

	const std::string& getName() const;

private:
	std::string m_Name;
};

} // namespace Game
