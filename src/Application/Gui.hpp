/**
 * @class Gui
 * @brief Gère l'interface graphique (GUI) à l'aide d'ImGui pour le projet MinePP.
 *
 * @details La classe Gui initialise le contexte ImGui, crée l’interface graphique et définit les fonctions pour démarrer
 *          et finaliser une frame GUI. Elle s'assure également de la bonne libération des ressources à la fermeture.
 *
 * @note Le singleton est utilisé pour s'assurer qu'une seule instance de Gui est active durant l'exécution.
 */


#pragma once

#include "../MinePP.hpp"

class Window;

class Gui
{
public:
	Gui(Window& window);
	~Gui();

	void beginFrame();
	void finalizeFrame();

	Gui(const Gui &) = delete;
	Gui(Gui &&) noexcept = delete;
	Gui(Gui &) = delete;
	Gui &operator=(Gui &) = delete;
	Gui &operator=(Gui &&) noexcept = delete;
};
