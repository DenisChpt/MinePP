#include "Application.hpp"

#include "../Utils/Utils.hpp"
#include "Window.hpp"
#include "../Core/Assets.hpp"

Application::Application(Window &window, Assets &assets)
	: window(window), assets(assets)
{
	window.setApplication(this);
}

Application::~Application()
{
}

int32_t Application::run()
{
	TRACE_FUNCTION();
	if (!scene || !window.isValid())
	{
		return -1;
	}

	lastTick = Clock::now();
	while (!window.shouldClose())
	{
		window.pollEvents();
		updateAndRender();
	}
	return 0;
}

void Application::updateAndRender()
{
	TRACE_FUNCTION();

	TimePoint now = Clock::now();
	float deltaTime = static_cast<float>((now - lastTick).count()) / 1000000000.0f;
	lastTick = now;

	scene->update(deltaTime);

	if (window.shouldRender())
	{
		TRACE_SCOPE("Window::render");
		window.beginFrame();
		scene->render();
		window.finalizeFrame();

		window.beginGuiFrame();
		scene->renderGui();
		window.finalizeGuiFrame();
	}

	window.swapBuffers();
}

void Application::onRefreshWindow()
{
	if (window.shouldRender())
	{
		window.beginFrame();
		scene->render();
		window.finalizeFrame();

		window.beginGuiFrame();
		scene->renderGui();
		window.finalizeGuiFrame();
	}

	window.swapBuffers();
}

void Application::onKeyEvent(int32_t key, int32_t scancode, int32_t action, int32_t mode)
{
	TRACE_FUNCTION();
	scene->onKeyEvent(key, scancode, action, mode);
}

void Application::onMouseButtonEvent(int32_t button, int32_t action, int32_t mods)
{
	TRACE_FUNCTION();
	scene->onMouseButtonEvent(button, action, mods);
}

void Application::onCursorPositionEvent(double x, double y)
{
	TRACE_FUNCTION();
	scene->onCursorPositionEvent(x, y);
}

void Application::onResized(int32_t width, int32_t height)
{
	TRACE_FUNCTION();
	scene->onResized(width, height);
}