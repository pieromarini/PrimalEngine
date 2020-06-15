#include "framebuffer.h"

#include "renderer.h"
#include "platform/openGL/openGLFramebuffer.h"

namespace primal {

  ref_ptr<Framebuffer> Framebuffer::create() {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLFramebuffer>();
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
