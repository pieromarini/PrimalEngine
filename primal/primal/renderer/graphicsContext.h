#pragma once

#include "../core/core.h"

namespace primal {

  class GraphicsContext {
	public:
	  virtual void init() = 0;
	  virtual void swapBuffers() = 0;

	  static scope_ptr<GraphicsContext> create(void* window);
  };

}
