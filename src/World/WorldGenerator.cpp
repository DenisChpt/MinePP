#include "WorldGenerator.hpp"
#include "WorldConstants.hpp"

WorldGenerator::WorldGenerator(int32_t seed) : seed(seed), noise(seed) {
	noise.SetFractalOctaves(WorldConstants::Noise::FractalOctaves);
	noise.SetFractalLacunarity(WorldConstants::Noise::FractalLacunarity);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);
}

void WorldGenerator::populateChunk(const Ref<Chunk>& chunkRef) {
	TRACE_FUNCTION();
	Chunk& chunk = *chunkRef;

	glm::ivec2 worldPosition = chunk.getPosition();
	glm::vec2 position = worldPosition;

	for (int32_t x = 0; x < Chunk::HorizontalSize; x++) {
		for (int32_t z = 0; z < Chunk::HorizontalSize; z++) {
			float noiseX = (position.x + static_cast<float>(x));
			float noiseY = (position.y + static_cast<float>(z));
			float noiseValue = noise.GetNoise(noiseX, noiseY) / WorldConstants::Noise::NormalizeScale + WorldConstants::Noise::NormalizeOffset;
			int32_t height = WorldConstants::Terrain::BaseHeight + static_cast<int32_t>(noiseValue * WorldConstants::Terrain::HeightVariation);

			for (int32_t y = 0; y < height; y++) {
				int32_t dy = height - y;
				BlockData::BlockType blockToPlace = BlockData::BlockType::stone;

				if (dy == WorldConstants::Layers::SurfaceLayerDepth) {
					if (y <= WorldConstants::Terrain::BeachUpperBound && y >= WorldConstants::Terrain::BeachLowerBound) {
						blockToPlace = BlockData::BlockType::sand;
					} else if (y < WorldConstants::Terrain::BeachLowerBound) {
						blockToPlace = BlockData::BlockType::stone;
					} else {
						blockToPlace = BlockData::BlockType::grass;
					}
				} else if (dy < WorldConstants::Layers::SubsurfaceLayerDepth) {
					if (y < WorldConstants::Terrain::SeaLevel) {
						blockToPlace = BlockData::BlockType::stone;
					} else {
						blockToPlace = BlockData::BlockType::dirt;
					}
				}

				chunk.placeBlock(blockToPlace, x, y, z);
			}

			for (int32_t y = WorldConstants::Terrain::SeaLevel; y >= height; y--) {
				chunk.placeBlock(BlockData::BlockType::water, x, y, z);
			}
			chunk.placeBlock(BlockData::BlockType::bedrock, x, WorldConstants::Terrain::BedrockLevel, z);
		}
	}
}
