#ifndef ICONFIG_HPP
#define ICONFIG_HPP

#include <string>

class IConfig {
public:
	virtual ~IConfig() = default;

	// Paramètres applicatifs
	virtual bool isDebug() const = 0;
	virtual bool isFullscreen() const = 0;
	virtual int getWindowWidth() const = 0;
	virtual int getWindowHeight() const = 0;
	virtual bool isVSync() const = 0;
	virtual double getScrollThreshold() const = 0;
	virtual int getMaxMessages() const = 0;
	virtual std::string getDbPath() const = 0;
	virtual bool useCache() const = 0;
	virtual int getDayLength() const = 0;
	virtual bool isInvertMouse() const = 0;

	// Options de rendu
	virtual bool showLights() const = 0;
	virtual bool showPlants() const = 0;
	virtual bool showClouds() const = 0;
	virtual bool showTrees() const = 0;
	virtual bool showItem() const = 0;
	virtual bool showCrosshairs() const = 0;
	virtual bool showWireframe() const = 0;
	virtual bool showInfoText() const = 0;
	virtual bool showChatText() const = 0;
	virtual bool showPlayerNames() const = 0;

	// Bindings clavier
	virtual char getKeyForward() const = 0;
	virtual char getKeyBackward() const = 0;
	virtual char getKeyLeft() const = 0;
	virtual char getKeyRight() const = 0;
	virtual int getKeyJump() const = 0;
	virtual int getKeyFly() const = 0;
	virtual char getKeyObserve() const = 0;
	virtual char getKeyObserveInset() const = 0;
	virtual char getKeyItemNext() const = 0;
	virtual char getKeyItemPrev() const = 0;
	virtual int getKeyZoom() const = 0;
	virtual char getKeyOrtho() const = 0;
	virtual char getKeyChat() const = 0;
	virtual char getKeyCommand() const = 0;
	virtual char getKeySign() const = 0;

	// Paramètres avancés
	virtual int getCreateChunkRadius() const = 0;
	virtual int getRenderChunkRadius() const = 0;
	virtual int getRenderSignRadius() const = 0;
	virtual int getDeleteChunkRadius() const = 0;
	virtual int getChunkSize() const = 0;
	virtual int getCommitInterval() const = 0;

	// Chargement futur depuis un JSON
	virtual bool loadFromJson(const std::string& jsonFilePath) = 0;
};

#endif // ICONFIG_HPP
