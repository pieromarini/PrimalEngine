#include <cstdint>

#include "buffer.h"
#include "renderer.h"
#include "rendererAPI.h"
#include "../../platform/openGL/openGLBuffer.h"

namespace primal {

  ref_ptr<VertexBuffer> VertexBuffer::create(float* vertices, uint32_t size) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLVertexBuffer>(vertices, size);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

  ref_ptr<IndexBuffer> IndexBuffer::create(uint32_t* indices, uint32_t size) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLIndexBuffer>(indices, size);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

}
