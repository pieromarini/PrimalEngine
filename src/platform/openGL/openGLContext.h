#pragma once

#include "../../primal/renderer/graphicsContext.h"

struct GLFWwindow;

namespace primal {

  class OpenGLContext : public GraphicsContext {
	public:
	  OpenGLContext(GLFWwindow* windowHandle);

	  virtual void init() override;
	  virtual void swapBuffers() override;
	private:
	  GLFWwindow* m_windowHandle;
  };

}
