#include "linuxWindow.h"

#include "../../primal/log.h"
#include "../../primal/events/applicationEvent.h"
#include "../../primal/events/mouseEvent.h"
#include "../../primal/events/keyEvent.h"
#include "../../primal/input.h"

#include "../openGL/openGLContext.h"

namespace primal {

  static bool s_GLFWInitialized = false;

  static void GLFWErrorCallback(int error, const char* description) {
	PRIMAL_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
  }

  Window* Window::create(const WindowProps& props) {
	return new LinuxWindow(props);
  }

  LinuxWindow::LinuxWindow(const WindowProps& props) {
	init(props);
  }

  LinuxWindow::~LinuxWindow() {
	shutdown();
  }

  void LinuxWindow::init(const WindowProps& props) {
	m_Data.title = props.title;
	m_Data.width = props.width;
	m_Data.height = props.height;

	PRIMAL_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

	if (!s_GLFWInitialized) {
	  // TODO: glfwTerminate on system shutdown
	  int success = glfwInit();
	  PRIMAL_CORE_ASSERT(success, "Could not intialize GLFW!");
	  glfwSetErrorCallback(GLFWErrorCallback);
	  s_GLFWInitialized = true;
	}

	m_Window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), m_Data.title.c_str(), nullptr, nullptr);

	m_Context = new OpenGLContext(m_Window);
	m_Context->init();

	glfwSetWindowUserPointer(m_Window, &m_Data);
	setVSync(true);

	// GLFW callbacks
	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
	  data.width = width;
	  data.height = height;

	  WindowResizeEvent event(width, height);
	  data.eventCallback(event);
	});

	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
	  WindowCloseEvent event;
	  data.eventCallback(event);
	});

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
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

	glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
	  WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

	  KeyTypedEvent event(static_cast<KeyCode>(keycode));
	  data.eventCallback(event);
	});

	glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
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

	glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
	  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

	  MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
	  data.eventCallback(event);
	});

	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
	  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

	  MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
	  data.eventCallback(event);
	});
  }

  void LinuxWindow::shutdown() {
	glfwDestroyWindow(m_Window);
  }

  void LinuxWindow::onUpdate() {
	glfwPollEvents();
	m_Context->swapBuffers();
  }

  void LinuxWindow::setVSync(bool enabled) {
	if (enabled)
	  glfwSwapInterval(1);
	else
	  glfwSwapInterval(0);

	m_Data.vSync = enabled;
  }

  bool LinuxWindow::isVSync() const {
	return m_Data.vSync;
  }

}
