#pragma once

#include "../MinePP.hpp"
#include "Chunk.hpp"

class WorldGenerator {
  int32_t seed;
  FastNoiseLite noise;

public:
  WorldGenerator(int32_t seed);
  [[nodiscard]] int32_t getSeed() const { return seed; };

  void populateChunk(const Ref<Chunk>& chunkRef);
};