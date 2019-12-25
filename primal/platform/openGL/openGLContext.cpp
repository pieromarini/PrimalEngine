#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "../../primal/core/log.h"
#include "openGLContext.h"

namespace primal {

  OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_windowHandle(windowHandle) {
	PRIMAL_CORE_ASSERT(windowHandle, "Window handle is null!")
  }

  void OpenGLContext::init() {

	glfwMakeContextCurrent(m_windowHandle);
	int status = gladLoadGL(glfwGetProcAddress);
	PRIMAL_CORE_ASSERT(status, "Failed to initialize Glad!");

	PRIMAL_CORE_INFO("OpenGL Info:");
	PRIMAL_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
	PRIMAL_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));

	int min, max;
	glGetIntegerv(GL_MAJOR_VERSION, &max);
	glGetIntegerv(GL_MINOR_VERSION, &min);

	PRIMAL_CORE_INFO("  Max Version: {0}", max);
	PRIMAL_CORE_INFO("  Min Version: {0}", min);

  }

  void OpenGLContext::swapBuffers() {
	glfwSwapBuffers(m_windowHandle);
  }

}
