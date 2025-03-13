/**
 * @class AssetRegistry
 * @brief Classe template abstraite pour le chargement et le cache des assets.
 *
 * @details Cette classe gère un unordered_map associant des noms d’assets à des pointeurs faibles.
 *          Elle définit la méthode pure loadAsset() qui doit être implémentée par les classes dérivées pour charger un asset spécifique.
 *
 * @tparam T Type de l'asset (ex. Texture, Shader, std::string, etc.).
 */


#pragma once

#include "../Rendering/ShaderProgram.hpp"
#include "../MinePP.hpp"

template <typename T>
class AssetRegistry
{
	std::unordered_map<std::string, WeakRef<const T>> registry;
	virtual Ref<const T> loadAsset(const std::string &name) = 0;

public:
	[[nodiscard]] bool hasAsset(const std::string &name) const
	{
		if (!registry.contains(name))
			return false;
		return !registry.at(name).expired();
	};

	void remove(const std::string &name)
	{
		if (hasAsset(name))
		{
			registry[name] = std::shared_ptr<T>();
		}
	}

	Ref<const T> get(const std::string &name)
	{
		if (hasAsset(name))
		{
			WeakRef<const T> ref = registry.at(name);
			return ref.lock();
		}

		Ref<const T> asset = loadAsset(name);
		if (asset != nullptr)
		{
			registry[name] = asset;
		}

		return asset;
	};

	virtual ~AssetRegistry() = default;
};