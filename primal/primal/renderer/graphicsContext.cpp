#include "graphicsContext.h"

#include "../renderer/renderer.h"
#include "../../platform/openGL/openGLContext.h"

namespace primal {

  scope_ptr<GraphicsContext> GraphicsContext::create(void* window) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
