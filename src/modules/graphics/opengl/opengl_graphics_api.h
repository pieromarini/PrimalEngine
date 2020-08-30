#ifndef OPENGL_GRAPHICSAPI_H
#define OPENGL_GRAPHICSAPI_H

#include "modules/graphics/graphics_api.h"

namespace primal {

  class OpenGLGraphicsAPI : public GraphicsAPI {
	  void init() override;
	  void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	  void setClearColor(const math::vec4& color) override;
	  void clear() override;

	  void clearColor() override;
	  void clearDepth() override;

	  /*
	  void drawIndexed(const Shared<VertexArray>& vertexArray) override;
	  void draw(const Shared<VertexArray>& vertexArray) override;
	  */

	  void setDepthTesting(bool flag) override;
  };

}

#endif
