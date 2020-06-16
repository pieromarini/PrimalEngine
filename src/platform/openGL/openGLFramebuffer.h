#pragma once

#include <glad/glad.h>

#include "primal/core/core.h"
#include "primal/renderer/framebuffer.h"

namespace primal {

  class OpenGLFramebuffer : public Framebuffer {
	public:
	  OpenGLFramebuffer(uint32_t width, uint32_t height);
	  ~OpenGLFramebuffer() override;

	  void bind() const override;
	  void unbind() const override;

	  void addTextureAttachment() override;
	  void addDepthAttachment() override;
	  void addDepthStencilAttachment() override;

	  void validate() const override;

	  void bindColorBufferTexture(uint32_t slot = 0) const override;
	  void bindDepthTexture(uint32_t slot = 0) const override;
	  void setDepthOnly() override;

	  [[nodiscard]] uint32_t getColorBufferTextureID() const override;
	  [[nodiscard]] uint32_t getDepthBufferTextureID() const override;

	private:
	  uint32_t m_id{};
	  uint32_t m_texture{};
	  uint32_t m_rbo{};
	  uint32_t m_depthTexture{};

	  uint32_t m_width;
	  uint32_t m_height;
  };

}
