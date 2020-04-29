#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "primal/core/log.h"
#include "primal/core/application.h"
#include "openGLContext.h"

namespace primal {

  OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_windowHandle(windowHandle) {
	PRIMAL_CORE_ASSERT(windowHandle, "Window handle is null!")
  }

  void OpenGLContext::init() {
	PRIMAL_PROFILE_FUNCTION();

	glfwMakeContextCurrent(m_windowHandle);
	int status = gladLoadGL();
	PRIMAL_CORE_ASSERT(status, "Failed to initialize Glad!");

	PRIMAL_CORE_INFO("OpenGL Info:");
	PRIMAL_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
	PRIMAL_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));

	int min = 0;
	int max = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &max);
	glGetIntegerv(GL_MINOR_VERSION, &min);

	PRIMAL_CORE_INFO("  Max Version: {0}", max);
	PRIMAL_CORE_INFO("  Min Version: {0}", min);

  }

  void OpenGLContext::swapBuffers() {
	PRIMAL_PROFILE_FUNCTION();

	glfwSwapBuffers(m_windowHandle);
  }

}
