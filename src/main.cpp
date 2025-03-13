/**
 * @file main.cpp
 * @brief Point d'entrée du programme MinePP.
 *
 * @details Ce fichier initialise l'application MinePP, démarre une session de trace de performances,
 *          crée une instance de la classe Application, configure la scène (par exemple via la lecture d’un fichier de sauvegarde)
 *          et lance la boucle principale de rendu.
 *
 * @param argc Nombre d'arguments de la ligne de commande.
 * @param argv Tableau des arguments.
 *
 * @return Code de retour de l'application.
 */


#include "Application/Application.hpp"
#include "Performance/Trace.hpp"
#include "MinePP.hpp"

int main(int argc, char **argv)
{
	START_TRACE("startup.json");
	auto *app = new Application;
	END_TRACE();

	START_TRACE("scene-creation.json");
	std::string savePath = argc > 1 ? argv[1] : "default.glc";
	app->setScene(std::make_shared<Scene>(savePath));
	END_TRACE();

	START_TRACE("runtime.json");
	int result;
	result = app->run();
	END_TRACE();

	START_TRACE("shutdown.json");
	delete app;
	END_TRACE();

	return result;
}
