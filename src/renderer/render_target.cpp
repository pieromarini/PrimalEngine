#include "render_target.h"
#include "tools/log.h"

namespace primal::renderer {

  RenderTarget::RenderTarget(unsigned int width, unsigned int height, GLenum type, unsigned int nrColorAttachments, bool depthAndStencil) {
	m_width = width;
	m_height = height;
	m_type = type;

	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	for (unsigned int i = 0; i < nrColorAttachments; ++i) {
	  Texture texture;
	  texture.m_filterMin = GL_LINEAR;
	  texture.m_filterMax = GL_LINEAR;
	  texture.m_wrapS = GL_CLAMP_TO_EDGE;
	  texture.m_wrapT = GL_CLAMP_TO_EDGE;
	  texture.m_mipmapping = false;

	  GLenum internalFormat = GL_RGBA;
	  if (type == GL_HALF_FLOAT)
		internalFormat = GL_RGBA16F;
	  else if (type == GL_FLOAT)
		internalFormat = GL_RGBA32F;
	  texture.generate(width, height, internalFormat, GL_RGBA, type, 0);

	  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture.m_id, 0);
	  m_ColorAttachments.push_back(texture);
	}
	// then generate Depth/Stencil texture if requested
	HasDepthAndStencil = depthAndStencil;
	if (depthAndStencil) {
	  Texture texture;
	  texture.m_filterMin = GL_LINEAR;
	  texture.m_filterMax = GL_LINEAR;
	  texture.m_wrapS = GL_CLAMP_TO_EDGE;
	  texture.m_wrapT = GL_CLAMP_TO_EDGE;
	  texture.m_mipmapping = false;
	  texture.generate(width, height, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

	  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.m_id, 0);
	  m_DepthStencil = texture;
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	  PRIMAL_CORE_ERROR("RenderTarget::RenderTarget: Framebuffer not complete.");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  Texture* RenderTarget::getDepthStencilTexture() {
	return &m_DepthStencil;
  }

  Texture* RenderTarget::getColorTexture(unsigned int index) {
	if (index < m_ColorAttachments.size())
	  return &m_ColorAttachments[index];
	else {
	  PRIMAL_CORE_WARN("RenderTarget::getColorTexture: RenderTarget color texture request, but not available {0}", index);
	  return nullptr;
	}
  }

  void RenderTarget::resize(unsigned int width, unsigned int height) {
	m_width = width;
	m_height = height;

	for (unsigned int i = 0; i < m_ColorAttachments.size(); ++i) {
	  m_ColorAttachments[i].resize(width, height);
	}
	// generate Depth/Stencil texture if requested
	if (HasDepthAndStencil) {
	  m_DepthStencil.resize(width, height);
	}
  }

  void RenderTarget::setTarget(GLenum target) {
	m_Target = target;
  }

  /* void RenderTarget::Bind(bool clear, bool setViewport)
	 {
	 glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	 if (clear)
	 {
	 if (HasDepthAndStencil)
	 {
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	 } 
	 else
	 {
	 glClear(GL_COLOR_BUFFER_BIT);
	 }
	 }
	 if (setViewport)
	 {
	 glViewport(0, 0, m_width, m_height);
	 }
	 }*/

}// namespace primal::renderer
