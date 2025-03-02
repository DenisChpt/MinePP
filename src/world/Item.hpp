#ifndef ITEM_HPP
#define ITEM_HPP

namespace World {

	// Définition des identifiants d'items/blocs
	constexpr int EMPTY        = 0;
	constexpr int GRASS        = 1;
	constexpr int SAND         = 2;
	constexpr int STONE        = 3;
	constexpr int BRICK        = 4;
	constexpr int WOOD         = 5;
	constexpr int CEMENT       = 6;
	constexpr int DIRT         = 7;
	constexpr int PLANK        = 8;
	constexpr int SNOW         = 9;
	constexpr int GLASS        = 10;
	constexpr int COBBLE       = 11;
	constexpr int LIGHT_STONE  = 12;
	constexpr int DARK_STONE   = 13;
	constexpr int CHEST        = 14;
	constexpr int LEAVES       = 15;
	constexpr int CLOUD        = 16;
	constexpr int TALL_GRASS   = 17;
	constexpr int YELLOW_FLOWER= 18;
	constexpr int RED_FLOWER   = 19;
	constexpr int PURPLE_FLOWER= 20;
	constexpr int SUN_FLOWER   = 21;
	constexpr int WHITE_FLOWER = 22;
	constexpr int BLUE_FLOWER  = 23;
	constexpr int COLOR_00     = 32;
	constexpr int COLOR_01     = 33;
	constexpr int COLOR_02     = 34;
	constexpr int COLOR_03     = 35;
	constexpr int COLOR_04     = 36;
	constexpr int COLOR_05     = 37;
	constexpr int COLOR_06     = 38;
	constexpr int COLOR_07     = 39;
	constexpr int COLOR_08     = 40;
	constexpr int COLOR_09     = 41;
	constexpr int COLOR_10     = 42;
	constexpr int COLOR_11     = 43;
	constexpr int COLOR_12     = 44;
	constexpr int COLOR_13     = 45;
	constexpr int COLOR_14     = 46;
	constexpr int COLOR_15     = 47;
	constexpr int COLOR_16     = 48;
	constexpr int COLOR_17     = 49;
	constexpr int COLOR_18     = 50;
	constexpr int COLOR_19     = 51;
	constexpr int COLOR_20     = 52;
	constexpr int COLOR_21     = 53;
	constexpr int COLOR_22     = 54;
	constexpr int COLOR_23     = 55;
	constexpr int COLOR_24     = 56;
	constexpr int COLOR_25     = 57;
	constexpr int COLOR_26     = 58;
	constexpr int COLOR_27     = 59;
	constexpr int COLOR_28     = 60;
	constexpr int COLOR_29     = 61;
	constexpr int COLOR_30     = 62;
	constexpr int COLOR_31     = 63;

	// Liste des items disponibles pour la construction.
	extern const int items[];
	extern const int item_count;

	// Tableau de correspondance pour chaque bloc (6 faces) – indices des tuiles.
	extern const int blocks[256][6];

	// Tableau associant certains ids à une tuile de plante (pour l'affichage des végétaux).
	extern const int plants[256];

	// Fonctions utilitaires pour vérifier les propriétés d'un item/bloc.
	bool isPlant(int w);
	bool isObstacle(int w);
	bool isTransparent(int w);
	bool isDestructable(int w);

} // namespace World

#endif // ITEM_HPP
