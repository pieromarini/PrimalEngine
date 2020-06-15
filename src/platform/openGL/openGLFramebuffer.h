#pragma once

#include <glad/glad.h>

#include "primal/core/core.h"
#include "primal/renderer/framebuffer.h"

namespace primal {

  class OpenGLFramebuffer : public Framebuffer {
	public:
	  OpenGLFramebuffer();
	  ~OpenGLFramebuffer() override;

	  void bind() const override;
	  void unbind() const override;

	  void addTextureAttachment() override;
	  void addDepthAttachment() override;
	  void addDepthStencilAttachment() override;

	  void validate() const override;

	  void bindColorBufferTexture() const override;
	  void setDepthOnly() override;

	private:
	  uint32_t m_id{};
	  uint32_t m_texture{};
	  uint32_t m_rbo{};
  };

}
