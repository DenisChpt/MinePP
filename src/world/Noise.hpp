#ifndef NOISE_HPP
#define NOISE_HPP

#include <vector>

/**
 * Classe de génération de bruit (Perlin Noise) en 3D.
 * Permet de générer des valeurs de bruit pour la génération procédurale de terrain.
 */
class Noise {
public:
	/**
	 * Constructeur.
	 * @param seed : graine utilisée pour initialiser le générateur de permutation.
	 */
	Noise(unsigned int seed = 0);

	/**
	 * Destructeur.
	 */
	~Noise();

	/**
	 * Génère une valeur de Perlin Noise en 3D.
	 * @param x : coordonnée x.
	 * @param y : coordonnée y.
	 * @param z : coordonnée z.
	 * @return une valeur de bruit entre -1 et 1.
	 */
	double perlinNoise(double x, double y, double z) const;

	/**
	 * Génère un bruit fractal (plusieurs octaves de Perlin Noise combinées).
	 * @param x : coordonnée x.
	 * @param y : coordonnée y.
	 * @param z : coordonnée z.
	 * @param octaves : nombre d'octaves.
	 * @param persistence : coefficient d'amplitude entre octaves.
	 * @param frequency : fréquence de base.
	 * @return une valeur de bruit normalisée.
	 */
	double octaveNoise(double x, double y, double z, int octaves, double persistence, double frequency) const;

private:
	std::vector<int> permutation; // Tableau de permutation utilisé pour le bruit.
	
	/**
	 * Initialise le tableau de permutation à partir d'une graine.
	 * @param seed : graine pour le générateur de nombres aléatoires.
	 */
	void initPermutation(unsigned int seed);

	/**
	 * Fonction d’atténuation (fade) de Ken Perlin.
	 * @param t : valeur d'entrée.
	 * @return valeur atténuée.
	 */
	double fade(double t) const;

	/**
	 * Interpolation linéaire entre a et b selon t.
	 */
	double lerp(double a, double b, double t) const;

	/**
	 * Calcule la contribution d’un sommet du cube.
	 * @param hash : valeur hashée.
	 * @param x, y, z : coordonnées relatives.
	 * @return contribution pour le calcul du bruit.
	 */
	double grad(int hash, double x, double y, double z) const;
};

#endif // NOISE_HPP
