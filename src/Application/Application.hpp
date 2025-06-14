/**
 * @class Application
 * @brief Gère le cycle de vie de l'application MinePP.
 *
 * @details La classe Application initialise la fenêtre, le système de rendu et la scène. Elle orchestre
 *          la boucle principale, gère la mise à jour et le rendu, et transmet les événements clavier, souris et redimensionnement
 *          aux modules concernés. Des macros de trace (TRACE_FUNCTION, etc.) sont utilisées pour mesurer les performances.
 *
 * @note La classe est implémentée en singleton via un pointeur statique.
 */


#pragma once

#include "../Scene/Scene.hpp"
#include "../Common.hpp"
#include "../Core/Context.hpp"

class Application
{
private:
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;
	using Clock = std::chrono::steady_clock;

	Context& context;
	Ref<Scene> scene;
	TimePoint lastTick = Clock::now();

	void onKeyEvent(int32_t key, int32_t scancode, int32_t action, int32_t mode);
	void onMouseButtonEvent(int32_t button, int32_t action, int32_t mods);
	void onCursorPositionEvent(double x, double y);
	void onResized(int32_t width, int32_t height);

	void onRefreshWindow();
	void updateAndRender();

	friend Window;

public:
	Application(Context& context);
	~Application();

	void setScene(const Ref<Scene> &newScene) { scene = newScene; };
	int32_t getWindowWidth() { return context.getWindow().getWindowWidth(); }
	int32_t getWindowHeight() { return context.getWindow().getWindowHeight(); }
	Window &getWindow() { return context.getWindow(); };
	int32_t run();

	Application(const Application &) = delete;
	Application(Application &) = delete;
	Application(Application &&) noexcept = delete;
	Application &operator=(Application &) = delete;
	Application &operator=(Application &&) noexcept = delete;
};
