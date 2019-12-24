#include "rendererAPI.h"

#include "../../platform/openGL/openGLRendererAPI.h"

namespace primal {

  RendererAPI::API RendererAPI::s_api = RendererAPI::API::OpenGL;

  scope_ptr<RendererAPI> RendererAPI::create() {
	switch (s_api) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createScope<OpenGLRendererAPI>();
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
