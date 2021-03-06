#include <glad/glad.h>

#include "core/core.h"
#include "tools/instrumentor.h"
#include "tools/log.h"
#include "opengl_graphics_api.h"

namespace primal {

  void openGLMessageCallback(unsigned  /*source*/, unsigned  /*type*/, unsigned  /*id*/, unsigned severity, int  /*length*/, const char* message, const void*  /*userParam*/) {
	switch (severity) {
	  case GL_DEBUG_SEVERITY_HIGH:         PRIMAL_CORE_CRITICAL(message); return;
	  case GL_DEBUG_SEVERITY_MEDIUM:       PRIMAL_CORE_ERROR(message); return;
	  case GL_DEBUG_SEVERITY_LOW:          PRIMAL_CORE_WARN(message); return;
	  case GL_DEBUG_SEVERITY_NOTIFICATION: PRIMAL_CORE_TRACE(message); return;
	}

	PRIMAL_CORE_ASSERT(false, "Unknown severity level!");
  }

  void OpenGLGraphicsAPI::init() {
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

  void OpenGLGraphicsAPI::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
  }

  void OpenGLGraphicsAPI::setClearColor(const math::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
  }

  void OpenGLGraphicsAPI::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void OpenGLGraphicsAPI::clearColor() {
	glClear(GL_COLOR_BUFFER_BIT);
  }

  void OpenGLGraphicsAPI::clearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
  }

  /*
  void OpenGLGraphicsAPI::drawIndexed(const Shared<VertexArray>& vertexArray) {
	vertexArray->bind();
	glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	vertexArray->unbind();
  }

  void OpenGLGraphicsAPI::draw(const Shared<VertexArray>& vertexArray) {

  }
  */

  void OpenGLGraphicsAPI::setDepthTesting(bool flag) {
	if (flag)
	  glEnable(GL_DEPTH_TEST);
	else
	  glDisable(GL_DEPTH_TEST);
  }

}
