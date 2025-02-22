#ifndef NOISE_HPP
#define NOISE_HPP

#include <vector>

/**
 * @brief Implements 3D Perlin noise and fractal noise functions.
 */
class Noise {
public:
	Noise(unsigned int seed = 0);
	~Noise();

	/**
	 * @brief Computes Perlin noise value in 3D space.
	 */
	double perlinNoise(double x, double y, double z) const;

	/**
	 * @brief Computes fractal noise by combining multiple octaves of Perlin noise.
	 */
	double octaveNoise(double x, double y, double z, int octaves, double persistence, double frequency) const;

private:
	std::vector<int> permutation;

	void initPermutation(unsigned int seed);
	double fade(double t) const;
	double lerp(double a, double b, double t) const;
	double grad(int hash, double x, double y, double z) const;
};

#endif // NOISE_HPP
