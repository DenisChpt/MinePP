#ifndef NOISE_HPP
#define NOISE_HPP

namespace Noise {

	// Initialise la graine pour le bruit.
	void seed(unsigned int x);

	// Génère un bruit simplex en 2D avec plusieurs octaves.
	float simplex2(float x, float y, int octaves, float persistence, float lacunarity);

	// Génère un bruit simplex en 3D avec plusieurs octaves.
	float simplex3(float x, float y, float z, int octaves, float persistence, float lacunarity);

} // namespace Noise

#endif // NOISE_HPP
