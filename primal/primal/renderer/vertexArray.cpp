#include "vertexArray.h"

#include "renderer.h"
#include "rendererAPI.h"
#include "../core/log.h"
#include "../../platform/openGL/openGLVertexArray.h"

namespace primal {

  VertexArray* VertexArray::create() {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return new OpenGLVertexArray();
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
