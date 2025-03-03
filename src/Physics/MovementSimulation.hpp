#pragma once

#include "../World/World.hpp"
#include "../MinePP.hpp"

namespace MovementSimulation {
  bool canMove(const glm::vec3 &from, const glm::vec3 &to, World &world);
};
