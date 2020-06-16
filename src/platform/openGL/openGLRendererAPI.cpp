#include <glad/glad.h>

#include "primal/core/core.h"
#include "primal/core/application.h"
#include "openGLRendererAPI.h"

namespace primal {

  void openGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam) {
	switch (severity) {
	  case GL_DEBUG_SEVERITY_HIGH:         PRIMAL_CORE_CRITICAL(message); return;
	  case GL_DEBUG_SEVERITY_MEDIUM:       PRIMAL_CORE_ERROR(message); return;
	  case GL_DEBUG_SEVERITY_LOW:          PRIMAL_CORE_WARN(message); return;
	  case GL_DEBUG_SEVERITY_NOTIFICATION: PRIMAL_CORE_TRACE(message); return;
	}

	PRIMAL_CORE_ASSERT(false, "Unknown severity level!");
  }

  void OpenGLRendererAPI::init() {
	PRIMAL_PROFILE_FUNCTION();

	PRIMAL_CORE_INFO("Enabling OpenGL Debug Output");
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLMessageCallback, nullptr);

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
  }

  void OpenGLRendererAPI::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
  }

  void OpenGLRendererAPI::setClearColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
  }

  void OpenGLRendererAPI::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void OpenGLRendererAPI::clearColor() {
	glClear(GL_COLOR_BUFFER_BIT);
  }

  void OpenGLRendererAPI::clearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
  }

  void OpenGLRendererAPI::drawIndexed(const ref_ptr<VertexArray>& vertexArray) {
	vertexArray->bind();
	glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	vertexArray->unbind();
  }

  void OpenGLRendererAPI::draw(const ref_ptr<VertexArray>& vertexArray) {

  }

  void OpenGLRendererAPI::setDepthTesting(bool flag) {
	if (flag)
	  glEnable(GL_DEPTH_TEST);
	else
	  glDisable(GL_DEPTH_TEST);
  }

}
