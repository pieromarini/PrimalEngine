#include "openGLFramebuffer.h"

#include "primal/core/application.h"
#include "primal/core/core.h"

namespace primal {

  OpenGLFramebuffer::OpenGLFramebuffer(uint32_t width, uint32_t height) : m_width{ width }, m_height{ height } {
	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  }

  OpenGLFramebuffer::~OpenGLFramebuffer() {
	glDeleteFramebuffers(1, &m_id);
  }

  void OpenGLFramebuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  }

  void OpenGLFramebuffer::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void OpenGLFramebuffer::addTextureAttachment() {
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glTextureStorage2D(m_texture, 1, GL_RGB8, m_width, m_height);

	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTextureUnit(0, m_texture);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
  }

  void OpenGLFramebuffer::addDepthAttachment() {
	glCreateTextures(GL_TEXTURE_2D, 1, &m_depthTexture);
	glTextureStorage2D(m_depthTexture, 1, GL_DEPTH_COMPONENT16, m_width, m_height);

	glTextureParameteri(m_depthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_depthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(m_depthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_depthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::array<float, 4> borderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(m_depthTexture, GL_TEXTURE_BORDER_COLOR, borderColor.data());

	glBindTextureUnit(0, m_depthTexture);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
  }

  void OpenGLFramebuffer::addDepthStencilAttachment() {
	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
  }

  void OpenGLFramebuffer::validate() const {
	glBindFramebuffer(GL_RENDERBUFFER, m_id);
	PRIMAL_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not valid");
	glBindFramebuffer(GL_RENDERBUFFER, 0);
  }

  void OpenGLFramebuffer::bindColorBufferTexture(uint32_t slot) const {
	glBindTextureUnit(slot, m_texture);
  }

  void OpenGLFramebuffer::bindDepthTexture(uint32_t slot) const {
	glBindTextureUnit(slot, m_depthTexture);
  }

  // NOTE: this allows us to have a framebuffer with ONLY a depth texture attachment
  void OpenGLFramebuffer::setDepthOnly() {
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
  }

  uint32_t OpenGLFramebuffer::getColorBufferTextureID() const {
	return m_texture;
  }

  uint32_t OpenGLFramebuffer::getDepthBufferTextureID() const {
	return m_depthTexture;
  }

}
