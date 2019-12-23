#include <glad/gl.h>

#include "openGLVertexArray.h"

namespace primal {

  static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
	switch (type) {
	  case primal::ShaderDataType::Float:    return GL_FLOAT;
	  case primal::ShaderDataType::Float2:   return GL_FLOAT;
	  case primal::ShaderDataType::Float3:   return GL_FLOAT;
	  case primal::ShaderDataType::Float4:   return GL_FLOAT;
	  case primal::ShaderDataType::Mat3:     return GL_FLOAT;
	  case primal::ShaderDataType::Mat4:     return GL_FLOAT;
	  case primal::ShaderDataType::Int:      return GL_INT;
	  case primal::ShaderDataType::Int2:     return GL_INT;
	  case primal::ShaderDataType::Int3:     return GL_INT;
	  case primal::ShaderDataType::Int4:     return GL_INT;
	  case primal::ShaderDataType::Bool:     return GL_BOOL;
	}

	PRIMAL_CORE_ASSERT(false, "Unknown ShaderDataType!");
	return 0;
  }

  OpenGLVertexArray::OpenGLVertexArray() {
	glCreateVertexArrays(1, &m_RendererID);
  }

  OpenGLVertexArray::~OpenGLVertexArray() {
	glDeleteVertexArrays(1, &m_RendererID);
  }

  void OpenGLVertexArray::bind() const {
	glBindVertexArray(m_RendererID);
  }

  void OpenGLVertexArray::unbind() const {
	glBindVertexArray(0);
  }

  void OpenGLVertexArray::addVertexBuffer(const ref_ptr<VertexBuffer>& vertexBuffer) {
	PRIMAL_CORE_ASSERT(vertexBuffer->getLayout().getElements().size(), "Vertex Buffer has no layout!");

	glBindVertexArray(m_RendererID);
	vertexBuffer->bind();

	uint32_t index = 0;
	const auto& layout = vertexBuffer->getLayout();
	for (const auto& element : layout) {
	  glEnableVertexAttribArray(index);
	  glVertexAttribPointer(index,
		  element.getComponentCount(),
		  ShaderDataTypeToOpenGLBaseType(element.type),
		  element.normalized ? GL_TRUE : GL_FALSE,
		  layout.getStride(),
		  (const void*)element.offset);
	  index++;
	}

	m_VertexBuffers.push_back(vertexBuffer);
  }

  void OpenGLVertexArray::setIndexBuffer(const ref_ptr<IndexBuffer>& indexBuffer) {
	glBindVertexArray(m_RendererID);
	indexBuffer->bind();

	m_IndexBuffer = indexBuffer;
  }

}
