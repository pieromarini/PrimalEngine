#pragma once

#include "primal/renderer/buffer.h"

namespace primal {

  class OpenGLVertexBuffer : public VertexBuffer {
	public:
	  OpenGLVertexBuffer(float* vertices, uint32_t size);
	  OpenGLVertexBuffer(const void* data, uint32_t size);
	  virtual ~OpenGLVertexBuffer();

	  virtual void bind() const override;
	  virtual void unbind() const override;

	  virtual const BufferLayout& getLayout() const override { return m_layout; }
	  virtual void setLayout(const BufferLayout& layout) override { m_layout = layout; }
	private:
	  uint32_t m_rendererID;
	  BufferLayout m_layout;
  };

  class OpenGLIndexBuffer : public IndexBuffer {
	public:
	  OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
	  virtual ~OpenGLIndexBuffer();

	  virtual void bind() const;
	  virtual void unbind() const;

	  virtual uint32_t getCount() const { return m_count; }
	private:
	  uint32_t m_rendererID;
	  uint32_t m_count;
  };

}
