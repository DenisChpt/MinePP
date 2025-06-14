#include "Window.hpp"

#include "../Utils/Utils.hpp"
#include "../Rendering/ColorRenderPass.hpp"
#include "../Core/Assets.hpp"
#include "Application.hpp"

Window::Window()
{
	TRACE_FUNCTION();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef BUILD_TYPE_DIST
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	window = glfwCreateWindow(windowWidth, windowHeight, name, nullptr, nullptr);
	glfwMakeContextCurrent(window);

	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		return;
	}

	if (!setupGlad())
	{
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		window = nullptr;
		return;
	}

	setupCallbacks();
	initGui();
}

Window::~Window()
{
	TRACE_FUNCTION();
	shutdownGui();
	glfwTerminate();
}

void Window::onWindowError(int32_t errorCode, const char *description)
{
	std::cerr << "GLFW: **ERROR** error=" << errorCode << " description=" << description << std::endl;
}

void Window::onKeyEvent(GLFWwindow *glfwWindow, int32_t key, int32_t scancode, int32_t action, int32_t mode)
{
	TRACE_FUNCTION();
	Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window && window->applicationPtr) {
		window->applicationPtr->onKeyEvent(key, scancode, action, mode);
	}
}

void Window::onResized(GLFWwindow *glfwWindow, int32_t width, int32_t height)
{
	TRACE_FUNCTION();
	Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window) {
		window->setWindowHeight(height);
		window->setWindowWidth(width);
		if (window->applicationPtr) {
			window->applicationPtr->onResized(width, height);
		}
	}
}

void Window::onMouseButtonEvent(GLFWwindow *glfwWindow, int32_t button, int32_t action, int32_t mods)
{
	TRACE_FUNCTION();
	Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window && window->applicationPtr) {
		window->applicationPtr->onMouseButtonEvent(button, action, mods);
	}
}

void Window::onCursorPosition(GLFWwindow *glfwWindow, double x, double y)
{
	TRACE_FUNCTION();
	Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window && window->applicationPtr) {
		window->applicationPtr->onCursorPositionEvent(x, y);
	}
}

void Window::onRefreshWindow(GLFWwindow *glfwWindow)
{
	TRACE_FUNCTION();
	Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window && window->applicationPtr) {
		window->applicationPtr->onRefreshWindow();
	}
}

void Window::setupCallbacks()
{
	TRACE_FUNCTION();
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, onKeyEvent);
	glfwSetMouseButtonCallback(window, onMouseButtonEvent);
	glfwSetCursorPosCallback(window, onCursorPosition);
	glfwSetFramebufferSizeCallback(window, onResized);

#ifndef BUILD_TYPE_DIST
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(onOpenGlMessage, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
	glfwSwapInterval(1);

	glfwSetWindowRefreshCallback(window, onRefreshWindow);
	glfwSetErrorCallback(Window::onWindowError);
}

bool Window::setupGlad()
{
	TRACE_FUNCTION();
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void Window::lockMouse()
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::unlockMouse()
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::pollEvents()
{
	TRACE_FUNCTION();
	glfwPollEvents();
}

bool Window::shouldRender()
{
	return windowWidth > 0 && windowHeight > 0;
}

void Window::beginFrame()
{
	TRACE_FUNCTION();
	assert(framebufferStack->empty());
	resetFrame(); // reset the default framebuffer

	static Ref<Framebuffer> framebuffer = nullptr;
	if (framebuffer == nullptr || framebuffer->getWidth() != windowWidth || framebuffer->getHeight() != windowHeight)
	{
		framebuffer = std::make_shared<Framebuffer>(windowWidth, windowHeight, true, 1);
	}

	framebufferStack->push(framebuffer);
	resetFrame(); // reset the level one framebuffer
}

void Window::resetFrame()
{
	TRACE_FUNCTION();
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Window::finalizeFrame()
{
	TRACE_FUNCTION();
	assert(framebufferStack->size() == 1);
	
	if (assetsPtr) {
		ColorRenderPass::renderTexture(framebufferStack->pop()->getColorAttachment(0), *assetsPtr);
	} else {
		// Fallback: just pop the framebuffer without rendering
		framebufferStack->pop();
	}
}

void Window::swapBuffers()
{
	TRACE_FUNCTION();
	framebufferStack->clearIntermediateTextureReferences();
	glfwSwapBuffers(window);
}

void GLAPIENTRY Window::onOpenGlMessage(GLenum source,
										GLenum type,
										GLuint id,
										GLenum severity,
										GLsizei,
										const GLchar *message,
										const void *)
{
	if (id == 131185 || id == 131218 || id == 131169 || id == 131076 || id == 131204)
	{
		return;
	}

	std::cerr << "---------------" << std::endl;
	std::cerr << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		std::cerr << "Source: API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		std::cerr << "Source: Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		std::cerr << "Source: Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		std::cerr << "Source: Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		std::cerr << "Source: Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		std::cerr << "Source: Other";
		break;
	default:
		break;
	}
	std::cerr << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		std::cerr << "Type: Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cerr << "Type: Deprecated Behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cerr << "Type: Undefined Behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cerr << "Type: Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cerr << "Type: Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		std::cerr << "Type: Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		std::cerr << "Type: Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		std::cerr << "Type: Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cerr << "Type: Other";
		break;
	default:
		break;
	}
	std::cerr << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cerr << "Severity: high";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cerr << "Severity: medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cerr << "Severity: low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cerr << "Severity: notification";
		break;
	default:
		break;
	}
	std::cerr << std::endl;
	std::cerr << std::endl;
}

glm::dvec2 Window::getCursorPosition()
{
	glm::dvec2 pos;
	glfwGetCursorPos(window, &pos.x, &pos.y);
	return pos;
}

void Window::initGui()
{
	TRACE_FUNCTION();

	if (window == nullptr)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
}

void Window::shutdownGui()
{
	TRACE_FUNCTION();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Window::beginGuiFrame()
{
	TRACE_FUNCTION();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Window::finalizeGuiFrame()
{
	TRACE_FUNCTION();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
