#pragma once
#include "INetworkManager.hpp"

namespace Game {

class NetworkManager : public INetworkManager {
public:
	NetworkManager();
	virtual ~NetworkManager();

	bool initialize() override;
	void update(float deltaTime) override;
	void shutdown() override;
};

} // namespace Game
