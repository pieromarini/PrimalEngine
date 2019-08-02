#pragma once

#include "../../primal/renderer/vertexArray.h"

namespace primal {

  class OpenGLVertexArray : public VertexArray {
	public:
	  OpenGLVertexArray();
	  virtual ~OpenGLVertexArray();

	  virtual void bind() const override;
	  virtual void unbind() const override;

	  virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
	  virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

	  virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const override { return m_VertexBuffers; }
	  virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const override { return m_IndexBuffer; }
	private:
	  uint32_t m_RendererID;
	  std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
	  std::shared_ptr<IndexBuffer> m_IndexBuffer;
  };

}
