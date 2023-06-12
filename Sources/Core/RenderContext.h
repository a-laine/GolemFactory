#pragma once

#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
#include <Math/TMath.h>


class RenderContext
{
	public:
		static RenderContext* getContextFromWindow(GLFWwindow* window);


		RenderContext(GLFWwindow* window, bool offScreen);

		// getters
		GLFWwindow* getParentWindow();
		const GLFWwindow* getParentWindow() const;
		vec2i getViewportSize() const;
		float getViewportRatio() const;

		// context related
		bool isOffScreenContext() const;
		bool isCurrentContext() const;
		void makeCurrent();
		void swapBuffers();
		void setVSync(bool enable);

		void updateViewportSize(vec2i size);

	private:
		GLFWwindow* m_window;
		bool m_isOffScreen;
		vec2i m_viewportSize;
};
