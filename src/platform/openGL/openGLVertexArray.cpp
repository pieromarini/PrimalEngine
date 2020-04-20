#include <glad/glad.h>

#include "../../primal/core/application.h"
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
	PRIMAL_PROFILE_FUNCTION();

	glCreateVertexArrays(1, &m_rendererID);
  }

  OpenGLVertexArray::~OpenGLVertexArray() {
	PRIMAL_PROFILE_FUNCTION();

	glDeleteVertexArrays(1, &m_rendererID);
  }

  void OpenGLVertexArray::bind() const {
	PRIMAL_PROFILE_FUNCTION();

	glBindVertexArray(m_rendererID);
  }

  void OpenGLVertexArray::unbind() const {
	PRIMAL_PROFILE_FUNCTION();

	glBindVertexArray(0);
  }

  void OpenGLVertexArray::addVertexBuffer(const ref_ptr<VertexBuffer>& vertexBuffer) {
	PRIMAL_PROFILE_FUNCTION();

	PRIMAL_CORE_ASSERT(vertexBuffer->getLayout().getElements().size(), "Vertex Buffer has no layout!");

	glBindVertexArray(m_rendererID);
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

	m_vertexBuffers.push_back(vertexBuffer);
  }

  void OpenGLVertexArray::setIndexBuffer(const ref_ptr<IndexBuffer>& indexBuffer) {
	PRIMAL_PROFILE_FUNCTION();

	glBindVertexArray(m_rendererID);
	indexBuffer->bind();

	m_indexBuffer = indexBuffer;
  }

}
