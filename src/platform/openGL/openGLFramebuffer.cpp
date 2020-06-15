#include "openGLFramebuffer.h"

#include "primal/core/application.h"
#include "primal/core/core.h"

namespace primal {

  OpenGLFramebuffer::OpenGLFramebuffer() {
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
	glTextureStorage2D(m_texture, 1, GL_RGB8, Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight());

	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTextureUnit(0, m_texture);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);  
  }

  void OpenGLFramebuffer::addDepthAttachment() {

  }

  void OpenGLFramebuffer::addDepthStencilAttachment() {
	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight());  

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
  }

  void OpenGLFramebuffer::validate() const {
	glBindFramebuffer(GL_RENDERBUFFER, m_id);
	PRIMAL_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not valid");
	glBindFramebuffer(GL_RENDERBUFFER, 0);
  }

  void OpenGLFramebuffer::bindColorBufferTexture() const {
	glBindTextureUnit(0, m_texture);
  }


  void OpenGLFramebuffer::setDepthOnly() {

  }

}
