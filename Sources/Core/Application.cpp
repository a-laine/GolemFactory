#include "Application.h"

#include <iostream>
#include <algorithm>

#include <Utiles/Assert.hpp>
#include <Resources/Loader/ImageLoader.h>


Application::Application() : m_mainWindow(nullptr), m_shouldExit(false)
{
	// Thibault PC : max opengl version is 4.6

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwSetErrorCallback(GLFWErrorCallback);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

Application::~Application()
{
	ImGuiShut();
	for (RenderContext* context : m_contexts)
	{
		delete context;
	}
	glfwTerminate();
}

void Application::initGLEW(int verbose)
{
	GF_ASSERT(m_mainWindow, "Create an OpenGL context before initializing GLEW");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "ERROR : " << glewGetErrorString(err) << std::endl;
		exitProgram(EXIT_FAILURE);
	}
	if (verbose) std::cout << "GLEW init success" << std::endl;
	if (verbose < 1) return;

	std::cout << "Status: GLEW version : " << glewGetString(GLEW_VERSION) << std::endl;

	std::cout << "        OpenGL version : " << glGetString(GL_VERSION) << std::endl;
	std::cout << "        OpenGL implementation vendor : " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "        Renderer name : " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "        GLSL version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void Application::setSouldExit(bool exit)
{
	m_shouldExit = exit;
}

bool Application::shouldExit()
{
	return m_shouldExit || (m_mainWindow && glfwWindowShouldClose(m_mainWindow));
}

void Application::exitProgram(int returnCode)
{
	ImGuiShut();
	glfwTerminate();
	exit(returnCode);
}

RenderContext* Application::createWindow(const char* title, int width, int height)
{
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, m_mainWindow);
	RenderContext* context = nullptr;

	if (window != nullptr)
	{
		context = new RenderContext(window, false);
		if (m_mainWindow == nullptr)
		{
			m_mainWindow = window;
			context->makeCurrent();

#ifdef USE_IMGUI
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 130");
#endif
		}

		m_windows.push_back(window);
		m_contexts.push_back(context);
	}

	return context;
}

RenderContext* Application::createFullscreenWindow(const char* title, int width, int height)
{
	return createFullscreenWindow(title, glfwGetPrimaryMonitor(), width, height);
}

RenderContext* Application::createFullscreenWindow(const char* title, GLFWmonitor* monitor, int width, int height)
{
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
	width = (width == 0) ? videoMode->width : width;
	height = (height == 0) ? videoMode->height : height;

	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, m_mainWindow);
	RenderContext* context = nullptr;

	if (window != nullptr)
	{
		context = new RenderContext(window, false);
		if (m_mainWindow == nullptr)
		{
			m_mainWindow = window;
			context->makeCurrent();

#ifdef USE_IMGUI
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 130");
#endif
		}
		m_windows.push_back(window);
		m_contexts.push_back(context);
	}

	return context;
}

RenderContext* Application::createOffscreenContext()
{
	if (!m_mainWindow)
		return nullptr;

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(80, 80, "", nullptr, m_mainWindow);
	RenderContext* context = nullptr;

	if (window != nullptr)
	{
		context = new RenderContext(window, true);
		m_windows.push_back(window);
		m_contexts.push_back(context);
	}

	return context;
}

void Application::closeWindow(GLFWwindow* window)
{
	if (m_mainWindow == window)
	{
		m_mainWindow = nullptr;
		m_shouldExit = true;
	}

	RenderContext* context = nullptr;
	auto itWindow = std::find(m_windows.begin(), m_windows.end(), window);
	if (itWindow != m_windows.end())
	{
		m_windows.erase(itWindow);
		context = RenderContext::getContextFromWindow(window);
		glfwDestroyWindow(window);
	}

	auto itContext = std::find(m_contexts.begin(), m_contexts.end(), context);
	if (itContext != m_contexts.end())
	{
		m_contexts.erase(itContext);
		delete context;
	}
}

void Application::maximizeMainWindow()
{
	glfwMaximizeWindow(m_mainWindow);
}

void Application::ImGuiShut()
{
#ifdef USE_IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif
}

void Application::changeIcon(const std::string& fullpath)
{
	int channel;
	GLFWimage icon;
	icon.pixels = ImageLoader::loadFromFile(fullpath, icon.width, icon.height, channel, ImageLoader::RGB_ALPHA);
	glfwSetWindowIcon(m_mainWindow, 1, &icon);
}

void Application::GLFWErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW ERROR : " << description << std::endl;
}
