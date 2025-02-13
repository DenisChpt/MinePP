#include "Item.hpp"
#include <iostream>

namespace Game {

Item::Item(const std::string& name)
	: m_Name(name)
{
}

Item::~Item() {
}

void Item::use() {
	std::cout << "[Item] Using item: " << m_Name << std::endl;
}

const std::string& Item::getName() const {
	return m_Name;
}

} // namespace Game
