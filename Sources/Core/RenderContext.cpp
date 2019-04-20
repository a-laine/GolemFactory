#include "RenderContext.h"

#include "Utiles/Assert.hpp"



RenderContext* RenderContext::getContextFromWindow(GLFWwindow* window)
{
	return static_cast<RenderContext*>(glfwGetWindowUserPointer(window));
}

RenderContext::RenderContext(GLFWwindow* window, bool offScreen)
	: m_window(window)
	, m_isOffScreen(offScreen)
{
	glfwSetWindowUserPointer(window, this);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	m_viewportSize = glm::ivec2(width, height);
}

GLFWwindow* RenderContext::getParentWindow()
{
	return m_window;
}

const GLFWwindow* RenderContext::getParentWindow() const
{
	return m_window;
}

void RenderContext::getViewportSize(glm::ivec2& size) const
{
	size = m_viewportSize;
}

float RenderContext::getViewportRatio() const
{
	glm::ivec2 size = m_viewportSize;
	return ((float) size.x) / ((float) size.y);
}

bool RenderContext::isOffScreenContext() const
{
	return m_isOffScreen;
}

bool RenderContext::isCurrentContext() const
{
	return glfwGetCurrentContext() == m_window;
}

void RenderContext::makeCurrent()
{
	glfwMakeContextCurrent(m_window);
}

void RenderContext::swapBuffers()
{
	GF_ASSERT(isCurrentContext());
	glfwSwapBuffers(m_window);
}

void RenderContext::setVSync(bool enable)
{
	GF_ASSERT(isCurrentContext());
	glfwSwapInterval(enable ? 1 : 0);
}

void RenderContext::updateViewportSize(glm::ivec2 size)
{
	if (isCurrentContext())
		glViewport(0, 0, size.x, size.y);
	m_viewportSize = size;
}
