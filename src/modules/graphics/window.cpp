#include "window.h"
#include "GLFW/glfw3.h"
#include "window_module.h"

namespace primal {

  WindowModule* Window::s_windowModule = nullptr;

  int Window::getWidth() {
	  int width;
	  glfwGetWindowSize(s_windowModule->m_windowHandle, &width, nullptr);
	  return width;
  }

  int Window::getHeight() {
	  int height;
	  glfwGetWindowSize(s_windowModule->m_windowHandle, nullptr, &height);
	  return height;
  }

  void Window::setTitle(const std::string_view title) {
	  glfwSetWindowTitle(s_windowModule->m_windowHandle, title.data());
  }

  void Window::setSizeLimits(float x, float y, float width, float height) {
	  glfwSetWindowSizeLimits(s_windowModule->m_windowHandle, x, y, width, height);
  }

  void Window::setFullscreen(bool fullscreen) {
	  if (isFullscreen() == fullscreen) return;
	  s_windowModule->setFullscreen(fullscreen);
  }

  bool Window::isFullscreen() {
	  return glfwGetWindowMonitor(s_windowModule->m_windowHandle);
  }

  void Window::setCursorMode(const CursorMode& mode) {
	  switch (mode) {
		case CursorMode::Normal: {
		  glfwSetInputMode(s_windowModule->m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		  break;
		}
		case CursorMode::Hidden: {
		  glfwSetInputMode(s_windowModule->m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		  break;
		}
		case CursorMode::Disabled: {
		  glfwSetInputMode(s_windowModule->m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		  break;
		}
	  }
  }

}
