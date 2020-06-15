#pragma once

#include <glm/glm.hpp>

#include "vertexArray.h"

namespace primal {

  class RendererAPI {
	public:
	  enum class API {
		None = 0, OpenGL = 1
	  };

	  virtual void init() = 0;
	  virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
	  virtual void setClearColor(const glm::vec4& color) = 0;
	  virtual void clear() = 0;

	  virtual void drawIndexed(const ref_ptr<VertexArray>& vertexArray) = 0;
	  virtual void draw(const ref_ptr<VertexArray>& vertexArray) = 0;

	  virtual void setDepthTesting(bool flag) = 0;

	  inline static API getAPI() { return s_api; }
	  static scope_ptr<RendererAPI> create();

	private:
	  static API s_api;
  };

}
