#include <glad/gl.h>

#include "../../primal/core/application.h"
#include "openGLBuffer.h"

namespace primal {

  /////////////////////////////////////////////////////////////////////////////
  // VertexBuffer /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
	PRIMAL_PROFILE_FUNCTION();

	glCreateBuffers(1, &m_rendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
  }

  OpenGLVertexBuffer::~OpenGLVertexBuffer() {
	PRIMAL_PROFILE_FUNCTION();

	glDeleteBuffers(1, &m_rendererID);
  }

  void OpenGLVertexBuffer::bind() const {
	PRIMAL_PROFILE_FUNCTION();

	glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
  }

  void OpenGLVertexBuffer::unbind() const {
	PRIMAL_PROFILE_FUNCTION();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  /////////////////////////////////////////////////////////////////////////////
  // IndexBuffer //////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
	: m_count(count) {
	PRIMAL_PROFILE_FUNCTION();

	glCreateBuffers(1, &m_rendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
  }

  OpenGLIndexBuffer::~OpenGLIndexBuffer() {
	PRIMAL_PROFILE_FUNCTION();

	glDeleteBuffers(1, &m_rendererID);
  }

  void OpenGLIndexBuffer::bind() const {
	PRIMAL_PROFILE_FUNCTION();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
  }

  void OpenGLIndexBuffer::unbind() const {
	PRIMAL_PROFILE_FUNCTION();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

}
