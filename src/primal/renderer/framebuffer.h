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

	  virtual void bindColorBufferTexture(uint32_t slot = 0) const = 0;
	  virtual void bindDepthTexture(uint32_t slot = 0) const = 0;
	  virtual void setDepthOnly() = 0;

	  [[nodiscard]] virtual uint32_t getColorBufferTextureID() const = 0;
	  [[nodiscard]] virtual uint32_t getDepthBufferTextureID() const = 0;

	  static ref_ptr<Framebuffer> create(uint32_t width, uint32_t height);
  };

}
