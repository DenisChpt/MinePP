#ifndef SIGN_HPP
#define SIGN_HPP

#include <string>
#include <vector>

namespace Utils {

// Longueur maximale d'un texte de panneau (sign)
constexpr size_t MAX_SIGN_LENGTH = 64;

struct Sign {
	int x;
	int y;
	int z;
	int face;
	std::string text; // On utilise std::string pour plus de flexibilité
};

class SignList {
public:
	// Constructeur par défaut avec une capacité initiale par défaut
	SignList();
	explicit SignList(size_t capacity);

	// Permet de réserver de la place (similaire à l'allocation initiale)
	void reserve(size_t capacity);

	// Ajoute un panneau : s'il existe déjà un panneau pour la même position et face, il est remplacé
	void add(int x, int y, int z, int face, const std::string &text);

	// Supprime le panneau situé aux coordonnées (x, y, z) pour la face donnée
	// Renvoie le nombre de panneaux supprimés (normalement 0 ou 1)
	int remove(int x, int y, int z, int face);

	// Supprime tous les panneaux situés aux coordonnées (x, y, z) (toutes faces confondues)
	// Renvoie le nombre de panneaux supprimés
	int remove_all(int x, int y, int z);

	// Accès en lecture sur la liste de panneaux
	const std::vector<Sign>& getSigns() const { return data_; }

	size_t size() const { return data_.size(); }

	void clear() { data_.clear(); }

private:
	std::vector<Sign> data_;
};

} // namespace Utils

#endif // SIGN_HPP
