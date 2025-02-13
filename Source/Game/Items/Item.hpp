#pragma once
#include <string>

namespace Game {

class Item {
public:
	Item(const std::string& name);
	virtual ~Item();

	virtual void use();
	const std::string& getName() const;

protected:
	std::string m_Name;
};

} // namespace Game
