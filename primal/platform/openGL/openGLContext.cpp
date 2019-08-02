#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "../../primal/log.h"
#include "openGLContext.h"

namespace primal {

  OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
	PRIMAL_CORE_ASSERT(windowHandle, "Window handle is null!")
  }

  void OpenGLContext::init() {
	glfwMakeContextCurrent(m_WindowHandle);
	int status = gladLoadGL(glfwGetProcAddress);
	PRIMAL_CORE_ASSERT(status, "Failed to initialize Glad!");

	PRIMAL_CORE_INFO("OpenGL Info:");
	PRIMAL_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
	PRIMAL_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
	PRIMAL_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));

  }

  void OpenGLContext::swapBuffers() {
	glfwSwapBuffers(m_WindowHandle);
  }

}
