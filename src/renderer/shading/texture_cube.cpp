#include "texture_cube.h"
#include <iostream>

namespace primal::renderer {

  TextureCube::TextureCube() {

  }

  TextureCube::~TextureCube() {

  }

  void TextureCube::defaultInitialize(uint32_t width, int32_t height, GLenum format, GLenum type, bool mipmap) {
	glGenTextures(1, &m_id);
	faceWidth = width;
	faceHeight = height;
	m_format = format;
	m_type = type;
	m_mipmapping = mipmap;
	
	if (type == GL_HALF_FLOAT && format == GL_RGB)
	  m_internalFormat = GL_RGB16F;
	else if (type == GL_FLOAT && format == GL_RGB)
	  m_internalFormat = GL_RGB32F;
	else if (type == GL_HALF_FLOAT && format == GL_RGBA)
	  m_internalFormat = GL_RGBA16F;
	else if (type == GL_FLOAT && format == GL_RGBA)
	  m_internalFormat = GL_RGBA32F;

	bind();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_filterMax);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_wrapT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_wrapR);

	for (std::size_t i = 0; i < 6; ++i) {
	  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_internalFormat, width, height, 0, format, type, nullptr);
	}

	if (mipmap)
	  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  }

  void TextureCube::generateFace(GLenum face, uint32_t width, uint32_t height, GLenum format, GLenum type, unsigned char* data) {
	if (faceWidth == 0)
	  glGenTextures(1, &m_id);

	faceWidth = width;
	faceHeight = height;
	m_format = format;
	m_type = type;

	bind();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_filterMax);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_wrapT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_wrapR);

	glTexImage2D(face, 0, format, width, height, 0, format, type, data);
  }

  void TextureCube::setMipFace(GLenum face, uint32_t width, uint32_t height, GLenum format, GLenum type, uint32_t mipLevel, unsigned char* data) {
	bind();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipLevel, 0, 0, width, height, format, type, data);
  }

  void TextureCube::resize(uint32_t width, uint32_t height) {
	faceWidth = width;
	faceHeight = height;

	bind();

	for (std::size_t i = 0; i < 6; ++i) {
	  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_format, width, height, 0, m_format, m_type, nullptr);
	}
  }

  void TextureCube::bind(int unit) {
	if (unit >= 0)
	  glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
  }

  void TextureCube::unbind() {
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

}
