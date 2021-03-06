#include "graphics_api.h"
#include "tools/log.h"

#include "opengl/opengl_graphics_api.h"

namespace primal {

  RenderingAPI GraphicsAPI::s_renderingAPI = RenderingAPI::OPENGL;

  Unique<GraphicsAPI> GraphicsAPI::create(RenderingAPI api) {
	switch (api) {
	  case RenderingAPI::OPENGL:
		return createUnique<OpenGLGraphicsAPI>();
	  case RenderingAPI::VULKAN:
	  case RenderingAPI::DIRECTX:
	  default:
		PRIMAL_CORE_CRITICAL("This API is not supported yet.");
		break;
	}

	return nullptr;
  }

}
