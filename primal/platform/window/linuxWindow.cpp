#include "linuxWindow.h"
#include <cstdlib>

#include "../../primal/core/application.h"
#include "../../primal/core/log.h"
#include "../../primal/events/applicationEvent.h"
#include "../../primal/events/mouseEvent.h"
#include "../../primal/events/keyEvent.h"
#include "../../primal/core/input.h"
#include "../../primal/renderer/renderer.h"
#include "../../primal/renderer/rendererAPI.h"

#include "../openGL/openGLContext.h"
#include <GLFW/glfw3.h>

namespace primal {

  static uint8_t s_glfwWindowCount = 0;

  static void GLFWErrorCallback(int error, const char* description) {
	PRIMAL_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
  }

  scope_ptr<Window> Window::create(const WindowProps& props) {
	return createScope<LinuxWindow>(props);
  }

  LinuxWindow::LinuxWindow(const WindowProps& props) {
	PRIMAL_PROFILE_FUNCTION();

	init(props);
  }

  LinuxWindow::~LinuxWindow() {
	PRIMAL_PROFILE_FUNCTION();

	shutdown();
  }

  void LinuxWindow::init(const WindowProps& props) {
	PRIMAL_PROFILE_FUNCTION();

	// NOTE: Require 4.6 version.
	auto e = setenv("MESA_GL_VERSION_OVERRIDE", "4.6", true);
	PRIMAL_CORE_ASSERT(e == 0, "Couldn't override OpenGL Version to 4.6");

	m_data.title = props.title;
	m_data.width = props.width;
	m_data.height = props.height;

	PRIMAL_CORE_INFO("Creating window {0} ({1} x {2})", props.title, props.width, props.height);

	if (s_glfwWindowCount == 0) {
	  PRIMAL_PROFILE_SCOPE("glfwInit");

	  int success = glfwInit();
	  PRIMAL_CORE_ASSERT(success, "Could not intialize GLFW!");
	  glfwSetErrorCallback(GLFWErrorCallback);
	}
#if defined(PRIMAL_DEBUG)
	if (Renderer::getAPI() == RendererAPI::API::OpenGL)
	  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	{
	  PRIMAL_PROFILE_SCOPE("glfwCreateWindow");
	  m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), m_data.title.c_str(), nullptr, nullptr);
	  s_glfwWindowCount++;
	}

	m_context = GraphicsContext::create(m_window);
	m_context->init();

	glfwSetWindowUserPointer(m_window, &m_data);
	setVSync(true);

	// GLFW callbacks
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
	  data.width = width;
	  data.height = height;

	  WindowResizeEvent event(width, height);
	  data.eventCallback(event);
	});

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
	  WindowCloseEvent event;
	  data.eventCallback(event);
	});

	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

	  switch (action) {
		case GLFW_PRESS: {
		  KeyPressedEvent event(static_cast<KeyCode>(key), 0);
		  data.eventCallback(event);
		  break;
		}
		case GLFW_RELEASE: {
		  KeyReleasedEvent event(static_cast<KeyCode>(key));
		  data.eventCallback(event);
		  break;
		}
		case GLFW_REPEAT: {
		  KeyPressedEvent event(static_cast<KeyCode>(key), 1);
		  data.eventCallback(event);
		  break;
		}
	  }
	});

	glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

	  KeyTypedEvent event(static_cast<KeyCode>(keycode));
	  data.eventCallback(event);
	});

	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

	  switch (action) {
		case GLFW_PRESS: {
		  MouseButtonPressedEvent event(static_cast<MouseCode>(button));
		  data.eventCallback(event);
		  break;
		}
		case GLFW_RELEASE: {
		  MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
		  data.eventCallback(event);
		  break;
		}
	  }
	});

	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
	  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

	  MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
	  data.eventCallback(event);
	});

	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
	  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

	  MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
	  data.eventCallback(event);
	});
  }

  void LinuxWindow::shutdown() {
	PRIMAL_PROFILE_FUNCTION();

	glfwDestroyWindow(m_window);
	s_glfwWindowCount--;

	if (s_glfwWindowCount == 0)
	  glfwTerminate();
  }

  void LinuxWindow::onUpdate() {
	PRIMAL_PROFILE_FUNCTION();

	glfwPollEvents();
	m_context->swapBuffers();
  }

  void LinuxWindow::setVSync(bool enabled) {
	PRIMAL_PROFILE_FUNCTION();

	if (enabled)
	  glfwSwapInterval(1);
	else
	  glfwSwapInterval(0);

	m_data.vSync = enabled;
  }

  bool LinuxWindow::isVSync() const {
	return m_data.vSync;
  }

}
