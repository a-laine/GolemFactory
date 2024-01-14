#pragma once

#include <vector>
#include <atomic>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "RenderContext.h"

#include "Utiles/ImguiConfig.h"

#include <string>


class Application
{
	public:
		Application();
		~Application();

		void initGLEW(int verbose);
		void setSouldExit(bool exit); // thread safe
		bool shouldExit();
		void exitProgram(int returnCode);

		RenderContext* createWindow(const char* title, int width, int height);
		RenderContext* createFullscreenWindow(const char* title, int width = 0, int height = 0);
		RenderContext* createFullscreenWindow(const char* title, GLFWmonitor* monitor, int width = 0, int height = 0);
		RenderContext* createOffscreenContext();
		void closeWindow(GLFWwindow* window);
		void maximizeMainWindow();

		void changeIcon(const std::string& iconName);


	private:
		static void GLFWErrorCallback(int error, const char* description);

		void ImGuiShut();

		std::vector<RenderContext*> m_contexts;
		std::vector<GLFWwindow*> m_windows;
		GLFWwindow* m_mainWindow;
		std::atomic_bool m_shouldExit;
};
