#include <glad/gl.h>

#include "openGLRendererAPI.h"

namespace primal {

  void OpenGLRendererAPI::setClearColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
  }

  void OpenGLRendererAPI::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void OpenGLRendererAPI::drawIndexed(const ref_ptr<VertexArray>& vertexArray) {
	glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
  }

}
