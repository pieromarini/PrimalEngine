#pragma once

#include "../../primal/renderer/vertexArray.h"

namespace primal {

  class OpenGLVertexArray : public VertexArray {
	public:
	  OpenGLVertexArray();
	  virtual ~OpenGLVertexArray();

	  virtual void bind() const override;
	  virtual void unbind() const override;

	  virtual void addVertexBuffer(const ref_ptr<VertexBuffer>& vertexBuffer) override;
	  virtual void setIndexBuffer(const ref_ptr<IndexBuffer>& indexBuffer) override;

	  virtual const std::vector<ref_ptr<VertexBuffer>>& getVertexBuffers() const override { return m_vertexBuffers; }
	  virtual const ref_ptr<IndexBuffer>& getIndexBuffer() const override { return m_indexBuffer; }
	private:
	  uint32_t m_rendererID;
	  std::vector<ref_ptr<VertexBuffer>> m_vertexBuffers;
	  ref_ptr<IndexBuffer> m_indexBuffer;
  };

}
