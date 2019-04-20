#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <atomic>



class RenderContext
{
	public:
		static RenderContext* getContextFromWindow(GLFWwindow* window);


		RenderContext(GLFWwindow* window, bool offScreen);

		// getters
		GLFWwindow* getParentWindow();
		const GLFWwindow* getParentWindow() const;
		void getViewportSize(glm::ivec2& size) const;
		float getViewportRatio() const;

		// context related
		bool isOffScreenContext() const;
		bool isCurrentContext() const;
		void makeCurrent();
		void swapBuffers();
		void setVSync(bool enable);

		void updateViewportSize(glm::ivec2 size);

	private:
		GLFWwindow* m_window;
		bool m_isOffScreen;
		std::atomic<glm::ivec2> m_viewportSize;
};
