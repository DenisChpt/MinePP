#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "IConfig.hpp"

class Config : public IConfig {
public:
	Config();

	// Paramètres applicatifs
	bool isDebug() const override;
	bool isFullscreen() const override;
	int getWindowWidth() const override;
	int getWindowHeight() const override;
	bool isVSync() const override;
	double getScrollThreshold() const override;
	int getMaxMessages() const override;
	std::string getDbPath() const override;
	bool useCache() const override;
	int getDayLength() const override;
	bool isInvertMouse() const override;

	// Options de rendu
	bool showLights() const override;
	bool showPlants() const override;
	bool showClouds() const override;
	bool showTrees() const override;
	bool showItem() const override;
	bool showCrosshairs() const override;
	bool showWireframe() const override;
	bool showInfoText() const override;
	bool showChatText() const override;
	bool showPlayerNames() const override;

	// Bindings clavier
	char getKeyForward() const override;
	char getKeyBackward() const override;
	char getKeyLeft() const override;
	char getKeyRight() const override;
	int getKeyJump() const override;
	int getKeyFly() const override;
	char getKeyObserve() const override;
	char getKeyObserveInset() const override;
	char getKeyItemNext() const override;
	char getKeyItemPrev() const override;
	int getKeyZoom() const override;
	char getKeyOrtho() const override;
	char getKeyChat() const override;
	char getKeyCommand() const override;
	char getKeySign() const override;

	// Paramètres avancés
	int getCreateChunkRadius() const override;
	int getRenderChunkRadius() const override;
	int getRenderSignRadius() const override;
	int getDeleteChunkRadius() const override;
	int getChunkSize() const override;
	int getCommitInterval() const override;

	// Chargement futur depuis un JSON
	bool loadFromJson(const std::string& jsonFilePath) override;

private:
	// Paramètres applicatifs
	bool debug;
	bool fullscreen;
	int windowWidth;
	int windowHeight;
	bool vsync;
	double scrollThreshold;
	int maxMessages;
	std::string dbPath;
	bool cache;
	int dayLength;
	bool invertMouse;

	// Options de rendu
	bool lights;
	bool plants;
	bool clouds;
	bool trees;
	bool item;
	bool crosshairs;
	bool wireframe;
	bool infoText;
	bool chatText;
	bool playerNames;

	// Bindings clavier
	char keyForward;
	char keyBackward;
	char keyLeft;
	char keyRight;
	int keyJump;
	int keyFly;
	char keyObserve;
	char keyObserveInset;
	char keyItemNext;
	char keyItemPrev;
	int keyZoom;
	char keyOrtho;
	char keyChat;
	char keyCommand;
	char keySign;

	// Paramètres avancés
	int createChunkRadius;
	int renderChunkRadius;
	int renderSignRadius;
	int deleteChunkRadius;
	int chunkSize;
	int commitInterval;
};

#endif // CONFIG_HPP
