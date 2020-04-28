#include "texture.h"

#include "renderer.h"
#include "platform/openGL/openGLTexture.h"

namespace primal {

  ref_ptr<Texture2D> Texture2D::create(uint32_t width, uint32_t height) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLTexture2D>(width, height);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

  ref_ptr<Texture2D> Texture2D::create(const std::string& path, const std::string& type) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLTexture2D>(path, type);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
