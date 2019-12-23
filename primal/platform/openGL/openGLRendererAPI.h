#pragma once

#include "../../primal/renderer/rendererAPI.h"

namespace primal {

  class OpenGLRendererAPI : public RendererAPI {
	public:
	  virtual void setClearColor(const glm::vec4& color) override;
	  virtual void clear() override;

	  virtual void drawIndexed(const ref_ptr<VertexArray>& vertexArray) override;
  };


}
