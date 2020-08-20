#include <stdexcept>

#include "window_module.h"
#include "application.h"
#include "tools/log.h"
#include "window.h"

namespace primal {
  
  void WindowModule::init() {
	glfwInit();
	initWindow();
	Window::s_windowModule = this;
  }

  void WindowModule::update(float deltaTime) {
	if (glfwWindowShouldClose(m_windowHandle)) {
	  Application::exit();
	}
	glfwSwapBuffers(m_windowHandle);
  }

  void WindowModule::shutdown() {
	glfwDestroyWindow(m_windowHandle);
	m_windowHandle = nullptr;
	glfwTerminate();
  }

  void WindowModule::initWindow() {
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// TODO: only enable if the renderingAPI is set to OpenGL.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// TODO: Temporary config. This should be moved to a global config loading context.
	auto windowConfig = WindowConfig {
	  .windowWidth = 1024,
	  .windowHeight = 768,
	  .windowTitle = "Primal Engine",
	  .windowFullscreen = 0,
	  .windowShowCursor = 1
	};

	if (windowConfig.windowFullscreen) {
	  auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	  m_windowHandle = glfwCreateWindow(mode->width, mode->height, windowConfig.windowTitle.c_str(), nullptr, nullptr);
	} else {
	  m_windowHandle = glfwCreateWindow(windowConfig.windowWidth, windowConfig.windowHeight, windowConfig.windowTitle.c_str(), nullptr, nullptr);
	}

	if (m_windowHandle == nullptr) {
	  glfwDestroyWindow(m_windowHandle);
	  m_windowHandle = glfwCreateWindow(800, 50, "Unable to initialize engine", nullptr, nullptr);
	  double startTime =  glfwGetTime();
	  while (glfwGetTime() - startTime < 5.0) { }
	  PRIMAL_CORE_CRITICAL("WindowModule::initWindow: Unable to initialize window.");
	}

	glfwSetWindowUserPointer(m_windowHandle, this);
	glfwMakeContextCurrent(m_windowHandle);

	glfwSwapInterval(0);
	glfwSetInputMode(m_windowHandle, GLFW_CURSOR, windowConfig.windowShowCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
  }

  void WindowModule::setFullscreen(bool fullscreen) {
	if (fullscreen) {
	  glfwGetWindowPos(m_windowHandle, &xPos, &yPos);
	  glfwGetWindowSize(m_windowHandle, &width, &height);
	  auto monitor = glfwGetPrimaryMonitor();
	  const auto mode = glfwGetVideoMode(monitor);
	  glfwSetWindowMonitor(m_windowHandle, monitor, xPos, yPos, width, height, mode->refreshRate);
	} else {
	  glfwSetWindowMonitor(m_windowHandle, nullptr, xPos, yPos, width, height, 0);
	}
  }

}
