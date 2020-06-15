#pragma once

#include "primal/core/core.h"

namespace primal {

  class Framebuffer {
	public:
	  virtual ~Framebuffer() = default;

	  virtual void bind() const = 0;
	  virtual void unbind() const = 0;

	  virtual void addTextureAttachment() = 0;
	  virtual void addDepthAttachment() = 0;
	  virtual void addDepthStencilAttachment() = 0;

	  virtual void validate() const = 0;

	  virtual void bindColorBufferTexture() const = 0;
	  virtual void setDepthOnly() = 0;

	  static ref_ptr<Framebuffer> create();
  };

}
