#pragma once

#include "framebuffer.h"
#include "primal/core/core.h"

namespace primal {

  class ShadowMap {
	public:
	  ShadowMap(uint32_t width, uint32_t height);

	  void bind();
	  void unbind();

	  void bindDepthTexture();
	  [[nodiscard]] uint32_t getDepthBufferTextureID() const;

	private:
	  ref_ptr<Framebuffer> m_framebuffer;
  };

}
