#pragma once

#include "primal/renderer/rendererAPI.h"

namespace primal {

  class OpenGLRendererAPI : public RendererAPI {
	public:
	  void init() override;
	  void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	  void setClearColor(const glm::vec4& color) override;
	  void clear() override;

	  void drawIndexed(const ref_ptr<VertexArray>& vertexArray) override;
  };


}
