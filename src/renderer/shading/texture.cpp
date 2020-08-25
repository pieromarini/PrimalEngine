#include "texture.h"
#include <cassert>

namespace primal::renderer {

  Texture::Texture() {

  }

  Texture::~Texture() {

  }

  void Texture::generate(uint32_t width, GLenum internalFormat, GLenum format, GLenum type, void* data) {
	glGenTextures(1, &m_id);
	m_width = width;
	m_height = 0;
	m_depth = 0;
	m_internalFormat = internalFormat;
	m_format = format;
	m_type = type;

	assert(m_target == GL_TEXTURE_1D);
	bind();

	glTexImage1D(m_target, 0, internalFormat, width, 0, format, type, data);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filterMax);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrapS);
	if (m_mipmapping)
	  glGenerateMipmap(m_target);

	unbind();
  }

  void Texture::generate(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum type, void* data) {
	glGenTextures(1, &m_id);
	m_width = width;
	m_height = height;
	m_depth = 0;
	m_internalFormat = internalFormat;
	m_format = format;
	m_type = type;

	assert(m_target == GL_TEXTURE_2D);
	bind();

	glTexImage2D(m_target, 0, internalFormat, width, height, 0, format, type, data);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filterMax);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_wrapT);
	if (m_mipmapping)
	  glGenerateMipmap(m_target);

	unbind();
  }

  void Texture::generate(uint32_t width, uint32_t height, uint32_t depth, GLenum internalFormat, GLenum format, GLenum type, void* data) {
	glGenTextures(1, &m_id);
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_internalFormat = internalFormat;
	m_format = format;
	m_type = type;

	assert(m_target == GL_TEXTURE_3D);
	bind();

	glTexImage3D(m_target, 0, internalFormat, width, height, depth, 0, format, type, data);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filterMax);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_wrapT);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_R, m_wrapR);
	if (m_mipmapping)
	  glGenerateMipmap(m_target);

	unbind();
  }

  void Texture::resize(uint32_t width, uint32_t height, uint32_t depth) {
	bind();

	if (m_target == GL_TEXTURE_1D) {
	  glTexImage1D(GL_TEXTURE_1D, 0, m_internalFormat, width, 0, m_format, m_type, nullptr);
	} else if (m_target == GL_TEXTURE_2D) {
	  glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0, m_format, m_type, nullptr);
	} else if (m_target == GL_TEXTURE_3D) {
	  glTexImage3D(GL_TEXTURE_3D, 0, m_internalFormat, width, height, depth, 0, m_format, m_type, nullptr);
	}
  }

  void Texture::bind(int unit) {
	if (unit >= 0)
	  glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(m_target, m_id);
  }

  void Texture::unbind() {
	glBindTexture(m_target, 0);
  }

  void Texture::setWrapMode(GLenum wrapMode, bool doBind) {
	if (doBind)
	  bind();

	if (m_target == GL_TEXTURE_1D) {
	  m_wrapS = wrapMode;
	  glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrapMode);
	} else if (m_target == GL_TEXTURE_2D) {
	  m_wrapS = wrapMode;
	  m_wrapT = wrapMode;
	  glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrapMode);
	  glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrapMode);
	} else if (m_target == GL_TEXTURE_3D) {
	  m_wrapS = wrapMode;
	  m_wrapT = wrapMode;
	  m_wrapR = wrapMode;
	  glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrapMode);
	  glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrapMode);
	  glTexParameteri(m_target, GL_TEXTURE_WRAP_R, wrapMode);
	}
  }

  void Texture::setFilterMin(GLenum filter, bool doBind) {
	if (doBind)
	  bind();

	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, filter);
  }

  void Texture::setFilterMax(GLenum filter, bool doBind) {
	if (doBind)
	  bind();

	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, filter);
  }

}
