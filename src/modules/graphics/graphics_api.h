#ifndef GRAPHICSAPI_H
#define GRAPHICSAPI_H

#include <glm/glm.hpp>

#include "core/memory.h"

namespace primal {

  enum class RenderingAPI {
	OPENGL = 0,
	VULKAN = 1,
	DIRECTX = 2
  };
  
  class GraphicsAPI {
	public:
	  virtual void init() = 0;
	  virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
	  virtual void setClearColor(const glm::vec4& color) = 0;
	  virtual void clear() = 0;

	  virtual void clearColor() = 0;
	  virtual void clearDepth() = 0;

	  /*
	  virtual void drawIndexed(const Shared<VertexArray>& vertexArray) = 0;
	  virtual void draw(const Shared<VertexArray>& vertexArray) = 0;
	  */

	  virtual void setDepthTesting(bool flag) = 0;

	  inline static RenderingAPI getAPI() { return s_renderingAPI; }

	  static Unique<GraphicsAPI> create(RenderingAPI api);

	private:
	  static RenderingAPI s_renderingAPI;
  };

}

#endif
