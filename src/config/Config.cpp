#include "Config.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

Config::Config()
	: debug(false),
	  fullscreen(false),
	  windowWidth(1024),
	  windowHeight(768),
	  vsync(true),
	  scrollThreshold(0.1),
	  maxMessages(4),
	  dbPath("MinePP.db"),
	  cache(true),
	  dayLength(600),
	  invertMouse(false),
	  lights(true),
	  plants(true),
	  clouds(true),
	  trees(true),
	  item(true),
	  crosshairs(true),
	  wireframe(true),
	  infoText(true),
	  chatText(true),
	  playerNames(true),
	  keyForward('W'),
	  keyBackward('S'),
	  keyLeft('A'),
	  keyRight('D'),
	  keyJump(GLFW_KEY_SPACE),
	  keyFly(GLFW_KEY_TAB),
	  keyObserve('O'),
	  keyObserveInset('P'),
	  keyItemNext('E'),
	  keyItemPrev('R'),
	  keyZoom(GLFW_KEY_LEFT_SHIFT),
	  keyOrtho('F'),
	  keyChat('t'),
	  keyCommand('/'),
	  keySign('`'),
	  createChunkRadius(10),
	  renderChunkRadius(10),
	  renderSignRadius(4),
	  deleteChunkRadius(14),
	  chunkSize(32),
	  commitInterval(5)
{
}

// Paramètres applicatifs
bool Config::isDebug() const { return debug; }
bool Config::isFullscreen() const { return fullscreen; }
int Config::getWindowWidth() const { return windowWidth; }
int Config::getWindowHeight() const { return windowHeight; }
bool Config::isVSync() const { return vsync; }
double Config::getScrollThreshold() const { return scrollThreshold; }
int Config::getMaxMessages() const { return maxMessages; }
std::string Config::getDbPath() const { return dbPath; }
bool Config::useCache() const { return cache; }
int Config::getDayLength() const { return dayLength; }
bool Config::isInvertMouse() const { return invertMouse; }

// Options de rendu
bool Config::showLights() const { return lights; }
bool Config::showPlants() const { return plants; }
bool Config::showClouds() const { return clouds; }
bool Config::showTrees() const { return trees; }
bool Config::showItem() const { return item; }
bool Config::showCrosshairs() const { return crosshairs; }
bool Config::showWireframe() const { return wireframe; }
bool Config::showInfoText() const { return infoText; }
bool Config::showChatText() const { return chatText; }
bool Config::showPlayerNames() const { return playerNames; }

// Bindings clavier
char Config::getKeyForward() const { return keyForward; }
char Config::getKeyBackward() const { return keyBackward; }
char Config::getKeyLeft() const { return keyLeft; }
char Config::getKeyRight() const { return keyRight; }
int Config::getKeyJump() const { return keyJump; }
int Config::getKeyFly() const { return keyFly; }
char Config::getKeyObserve() const { return keyObserve; }
char Config::getKeyObserveInset() const { return keyObserveInset; }
char Config::getKeyItemNext() const { return keyItemNext; }
char Config::getKeyItemPrev() const { return keyItemPrev; }
int Config::getKeyZoom() const { return keyZoom; }
char Config::getKeyOrtho() const { return keyOrtho; }
char Config::getKeyChat() const { return keyChat; }
char Config::getKeyCommand() const { return keyCommand; }
char Config::getKeySign() const { return keySign; }

// Paramètres avancés
int Config::getCreateChunkRadius() const { return createChunkRadius; }
int Config::getRenderChunkRadius() const { return renderChunkRadius; }
int Config::getRenderSignRadius() const { return renderSignRadius; }
int Config::getDeleteChunkRadius() const { return deleteChunkRadius; }
int Config::getChunkSize() const { return chunkSize; }
int Config::getCommitInterval() const { return commitInterval; }

// Chargement futur depuis un JSON (stub)
bool Config::loadFromJson(const std::string& jsonFilePath) {
	std::cerr << "loadFromJson n'est pas encore implémenté." << std::endl;
	return false;
}
