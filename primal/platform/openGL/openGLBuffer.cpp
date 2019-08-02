#include <glad/gl.h>

#include "openGLBuffer.h"

namespace primal {

  /////////////////////////////////////////////////////////////////////////////
  // VertexBuffer /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
  }

  OpenGLVertexBuffer::~OpenGLVertexBuffer() {
	glDeleteBuffers(1, &m_RendererID);
  }

  void OpenGLVertexBuffer::bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
  }

  void OpenGLVertexBuffer::unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  /////////////////////////////////////////////////////////////////////////////
  // IndexBuffer //////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
	: m_Count(count) {

	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
  }

  OpenGLIndexBuffer::~OpenGLIndexBuffer() {
	glDeleteBuffers(1, &m_RendererID);
  }

  void OpenGLIndexBuffer::bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
  }

  void OpenGLIndexBuffer::unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

}
