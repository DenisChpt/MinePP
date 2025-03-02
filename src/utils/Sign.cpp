#include "Sign.hpp"

namespace Utils {

SignList::SignList() {
	// Capacité par défaut
	data_.reserve(16);
}

SignList::SignList(size_t capacity) {
	data_.reserve(capacity);
}

void SignList::reserve(size_t capacity) {
	data_.reserve(capacity);
}

void SignList::add(int x, int y, int z, int face, const std::string &text) {
	// On supprime d'abord le panneau existant pour la même position et face
	remove(x, y, z, face);
	Sign sign;
	sign.x = x;
	sign.y = y;
	sign.z = z;
	sign.face = face;
	// On limite la longueur du texte à MAX_SIGN_LENGTH caractères
	if (text.size() > MAX_SIGN_LENGTH) {
		sign.text = text.substr(0, MAX_SIGN_LENGTH);
	} else {
		sign.text = text;
	}
	data_.push_back(sign);
}

int SignList::remove(int x, int y, int z, int face) {
	int count = 0;
	// On parcourt la liste et on supprime (en échangeant avec le dernier élément)
	for (size_t i = 0; i < data_.size();) {
		if (data_[i].x == x && data_[i].y == y && data_[i].z == z && data_[i].face == face) {
			data_[i] = std::move(data_.back());
			data_.pop_back();
			count++;
			// On ne fait pas i++ pour réexaminer l'élément échangé
		} else {
			++i;
		}
	}
	return count;
}

int SignList::remove_all(int x, int y, int z) {
	int count = 0;
	for (size_t i = 0; i < data_.size();) {
		if (data_[i].x == x && data_[i].y == y && data_[i].z == z) {
			data_[i] = std::move(data_.back());
			data_.pop_back();
			count++;
		} else {
			++i;
		}
	}
	return count;
}

} // namespace Utils
