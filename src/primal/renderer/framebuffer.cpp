#include "framebuffer.h"

#include "renderer.h"
#include "platform/openGL/openGLFramebuffer.h"

namespace primal {

  ref_ptr<Framebuffer> Framebuffer::create(uint32_t width, uint32_t height) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLFramebuffer>(width, height);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
