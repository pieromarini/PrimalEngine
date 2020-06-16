#include "shadowMap.h"

namespace primal {

  ShadowMap::ShadowMap(uint32_t width, uint32_t height) {
	m_framebuffer = Framebuffer::create(width, height);
	m_framebuffer->addDepthAttachment();
	m_framebuffer->setDepthOnly();
	m_framebuffer->validate();
  }

  void ShadowMap::bind() {
	m_framebuffer->bind();
  }

  void ShadowMap::unbind() {
	m_framebuffer->unbind();
  }

  void ShadowMap::bindDepthTexture() {
	m_framebuffer->bindDepthTexture(8);
  }

  uint32_t ShadowMap::getDepthBufferTextureID() const {
	return m_framebuffer->getDepthBufferTextureID();	
  }

}
