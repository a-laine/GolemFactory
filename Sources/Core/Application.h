#pragma once

#include <vector>
#include <atomic>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <Core/RenderContext.h>



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

private:
	static void GLFWErrorCallback(int error, const char* description);

	std::vector<RenderContext*> m_contexts;
	std::vector<GLFWwindow*> m_windows;
	GLFWwindow* m_mainWindow;
	std::atomic_bool m_shouldExit;
};
