#pragma once

namespace Game {

class INetworkManager {
public:
	virtual ~INetworkManager() = default;
	virtual bool initialize() = 0;
	virtual void update(float deltaTime) = 0;
	virtual void shutdown() = 0;
};

} // namespace Game
