#ifndef MAP_HPP
#define MAP_HPP

#include <vector>
#include <cstdint>
#include <functional>

namespace World {

// Représente une entrée dans la table de hachage.
// La valeur brute est accessible via "value" et, sous forme structurée, via "e".
struct MapEntry {
	union {
		uint32_t value;
		struct {
			uint8_t x;
			uint8_t y;
			uint8_t z;
			int8_t w; // Valeur associée (par exemple, un type de bloc)
		} e;
	};

	MapEntry() : value(0) {} // Une entrée vide a une valeur nulle.
};

// Classe implémentant une table de hachage adaptée aux coordonnées 3D.
class Map {
public:
	// Construit une Map avec une translation (dx,dy,dz) et une taille initiale donnée par mask.
	// mask doit être de la forme (2^n - 1) (exemple : 15, 31, etc.), la taille du tableau sera mask+1.
	Map(int dx, int dy, int dz, unsigned int mask);

	// Constructeur de copie et opérateur d'affectation.
	Map(const Map &other);
	Map& operator=(const Map &other);

	// Définit la valeur associée aux coordonnées (x, y, z) à w.
	// Renvoie 1 si la valeur a été modifiée ou insérée, 0 sinon.
	int set(int x, int y, int z, int w);

	// Récupère la valeur associée aux coordonnées (x, y, z), ou 0 si aucune entrée n'existe.
	int get(int x, int y, int z) const;

	// Parcourt toutes les entrées non vides et appelle la fonction "func" avec les coordonnées absolues et la valeur.
	void forEach(std::function<void(int ex, int ey, int ez, int ew)> func) const;

	int getDx() const { return dx_; }
	int getDy() const { return dy_; }
	int getDz() const { return dz_; }

	std::vector<MapEntry> getData() const { return data_; }

	unsigned int getSize() const { return size_; }

	void clear();
private:
	// Agrandit la table de hachage (lorsque le nombre d'entrées devient trop élevé).
	void grow();

	// Fonction de hachage d'un entier.
	static int hash_int(int key);
	// Combine les trois valeurs pour obtenir un hachage.
	static int hash(int x, int y, int z);

	int dx_, dy_, dz_;        // Décalages de translation (les coordonnées réelles = coordonnées stockées + dx, dy, dz).
	unsigned int mask_;        // Masque utilisé pour la taille de la table (taille = mask_ + 1).
	unsigned int size_;        // Nombre d'entrées non vides.
	std::vector<MapEntry> data_; // Tableau de MapEntry.
};

} // namespace World

#endif // MAP_HPP
