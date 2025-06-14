/**
 * @file WorldConstants.hpp
 * @brief Constants for world generation parameters
 * 
 * @details Centralizes all magic numbers used in world generation
 *          to improve code readability and maintainability
 */

#pragma once

#include <cstdint>

namespace WorldConstants {
	/**
	 * @brief Noise generation parameters
	 */
	namespace Noise {
		constexpr int32_t FractalOctaves = 5;
		constexpr float FractalLacunarity = 1.75f;
		constexpr float NormalizeOffset = 0.5f;
		constexpr float NormalizeScale = 2.0f;
	}

	/**
	 * @brief Terrain height parameters
	 */
	namespace Terrain {
		/**
		 * @brief Base height of the terrain (minimum height)
		 */
		constexpr int32_t BaseHeight = 45;
		
		/**
		 * @brief Maximum height variation from noise
		 */
		constexpr int32_t HeightVariation = 45;
		
		/**
		 * @brief Sea level height
		 */
		constexpr int32_t SeaLevel = 64;
		
		/**
		 * @brief Beach sand level boundaries
		 */
		constexpr int32_t BeachUpperBound = 64;
		constexpr int32_t BeachLowerBound = 63;
		
		/**
		 * @brief Bedrock layer height
		 */
		constexpr int32_t BedrockLevel = 0;
	}

	/**
	 * @brief Layer depth parameters
	 */
	namespace Layers {
		/**
		 * @brief Depth of the surface layer (grass/sand)
		 */
		constexpr int32_t SurfaceLayerDepth = 1;
		
		/**
		 * @brief Depth of the subsurface layer (dirt under grass)
		 */
		constexpr int32_t SubsurfaceLayerDepth = 5;
	}
}