#pragma once

#include <memory>
#include <vector>

#include "buffer.h"

namespace primal {

  class VertexArray {
	public:
	  virtual ~VertexArray() {}

	  virtual void bind() const = 0;
	  virtual void unbind() const = 0;

	  virtual void addVertexBuffer(const ref_ptr<VertexBuffer>& vertexBuffer) = 0;
	  virtual void setIndexBuffer(const ref_ptr<IndexBuffer>& indexBuffer) = 0;

	  virtual const std::vector<ref_ptr<VertexBuffer>>& getVertexBuffers() const = 0;
	  virtual const ref_ptr<IndexBuffer>& getIndexBuffer() const = 0;

	  static ref_ptr<VertexArray> create();
  };

}
