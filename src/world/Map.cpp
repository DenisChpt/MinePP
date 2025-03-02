#include "Map.hpp"
#include <cstdlib>
#include <cstring>

namespace World {

int Map::hash_int(int key) {
	key = ~key + (key << 15);
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057;
	key = key ^ (key >> 16);
	return key;
}

int Map::hash(int x, int y, int z) {
	x = hash_int(x);
	y = hash_int(y);
	z = hash_int(z);
	return x ^ y ^ z;
}

Map::Map(int dx, int dy, int dz, unsigned int mask)
	: dx_(dx), dy_(dy), dz_(dz), mask_(mask), size_(0), data_(mask + 1)
{
	// Les MapEntry sont initialisées avec value = 0 grâce au constructeur par défaut.
}

Map::Map(const Map &other)
	: dx_(other.dx_), dy_(other.dy_), dz_(other.dz_),
	  mask_(other.mask_), size_(other.size_), data_(other.data_)
{
}

Map& Map::operator=(const Map &other) {
	if (this != &other) {
		dx_ = other.dx_;
		dy_ = other.dy_;
		dz_ = other.dz_;
		mask_ = other.mask_;
		size_ = other.size_;
		data_ = other.data_;
	}
	return *this;
}

int Map::set(int x, int y, int z, int w) {
	unsigned int index = hash(x, y, z) & mask_;
	// On calcule les coordonnées relatives par rapport aux décalages.
	int relX = x - dx_;
	int relY = y - dy_;
	int relZ = z - dz_;
	MapEntry *entry = &data_[index];
	int overwrite = 0;
	while (entry->value != 0) {
		if (entry->e.x == static_cast<uint8_t>(relX) &&
			entry->e.y == static_cast<uint8_t>(relY) &&
			entry->e.z == static_cast<uint8_t>(relZ)) {
			overwrite = 1;
			break;
		}
		index = (index + 1) & mask_;
		entry = &data_[index];
	}
	if (overwrite) {
		if (entry->e.w != w) {
			entry->e.w = static_cast<int8_t>(w);
			return 1;
		}
	} else if (w != 0) {
		entry->e.x = static_cast<uint8_t>(relX);
		entry->e.y = static_cast<uint8_t>(relY);
		entry->e.z = static_cast<uint8_t>(relZ);
		entry->e.w = static_cast<int8_t>(w);
		size_++;
		// On agrandit la table si plus de la moitié est occupée.
		if (size_ * 2 > mask_) {
			grow();
		}
		return 1;
	}
	return 0;
}

int Map::get(int x, int y, int z) const {
	unsigned int index = hash(x, y, z) & mask_;
	int relX = x - dx_;
	int relY = y - dy_;
	int relZ = z - dz_;
	// Vérification de la validité des coordonnées relatives (dans l'intervalle [0,255])
	if (relX < 0 || relX > 255) return 0;
	if (relY < 0 || relY > 255) return 0;
	if (relZ < 0 || relZ > 255) return 0;
	const MapEntry *entry = &data_[index];
	while (entry->value != 0) {
		if (entry->e.x == static_cast<uint8_t>(relX) &&
			entry->e.y == static_cast<uint8_t>(relY) &&
			entry->e.z == static_cast<uint8_t>(relZ)) {
			return entry->e.w;
		}
		index = (index + 1) & mask_;
		entry = &data_[index];
	}
	return 0;
}

void Map::grow() {
	// Création d'une nouvelle table avec une taille doublée.
	Map newMap(dx_, dy_, dz_, (mask_ << 1) | 1);
	// Réinsertion de toutes les entrées non vides dans la nouvelle table.
	for (unsigned int i = 0; i < data_.size(); i++) {
		const MapEntry &entry = data_[i];
		if (entry.value == 0) continue;
		int ex = static_cast<int>(entry.e.x) + dx_;
		int ey = static_cast<int>(entry.e.y) + dy_;
		int ez = static_cast<int>(entry.e.z) + dz_;
		int ew = entry.e.w;
		newMap.set(ex, ey, ez, ew);
	}
	// Remplace la table actuelle par la nouvelle.
	*this = std::move(newMap);
}

void Map::forEach(std::function<void(int ex, int ey, int ez, int ew)> func) const {
	for (unsigned int i = 0; i < data_.size(); i++) {
		const MapEntry &entry = data_[i];
		if (entry.value == 0) continue;
		int ex = static_cast<int>(entry.e.x) + dx_;
		int ey = static_cast<int>(entry.e.y) + dy_;
		int ez = static_cast<int>(entry.e.z) + dz_;
		int ew = entry.e.w;
		func(ex, ey, ez, ew);
	}
}

void Map::clear() {
	size_ = 0;
	data_.assign(mask_ + 1, MapEntry());
}

} // namespace World
