#pragma once

#include "primal/renderer/graphicsContext.h"

struct GLFWwindow;

namespace primal {

  class OpenGLContext : public GraphicsContext {
	public:
	  OpenGLContext(GLFWwindow* windowHandle);

	  void init() override;
	  void swapBuffers() override;

	private:
	  GLFWwindow* m_windowHandle;
  };

}
