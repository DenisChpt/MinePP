#include "Gui.hpp"

#include "../Utils/Utils.hpp"
#include "Window.hpp"

Gui::Gui(Window& window)
{
	TRACE_FUNCTION();

	const auto context = window.getContext();
	if (context == nullptr)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(context, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
}

Gui::~Gui()
{
	TRACE_FUNCTION();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Gui::beginFrame()
{
	TRACE_FUNCTION();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Gui::finalizeFrame()
{
	TRACE_FUNCTION();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
