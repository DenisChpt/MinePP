#pragma once
#include "../Item.hpp"

namespace Game {

class Pickaxe : public Item {
public:
	Pickaxe();
	virtual ~Pickaxe();

	void use() override;
};

} // namespace Game
