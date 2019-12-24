#include "openGLTexture.h"

#include <SOIL/SOIL.h>

namespace primal {

  OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
	: m_width(width), m_height(height) {

	m_internalFormat = GL_RGBA8;
	m_dataFormat = GL_RGBA;

	glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
	glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

	glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_path(path) {

	int width, height, channels;
	// stbi_set_flip_vertically_on_load(1);
	auto data = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

	PRIMAL_CORE_ASSERT(data, "Failed to load image!");

	m_width = width;
	m_height = height;

	GLenum internalFormat = 0, dataFormat = 0;
	if (channels == 4) {
	  internalFormat = GL_RGBA8;
	  dataFormat = GL_RGBA;
	} else if (channels == 3) {
	  internalFormat = GL_RGB8;
	  dataFormat = GL_RGB;
	}

	m_internalFormat = internalFormat;
	m_dataFormat = dataFormat;

	PRIMAL_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

	glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
	glTextureStorage2D(m_rendererID, 1, internalFormat, m_width, m_height);

	glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, dataFormat, GL_UNSIGNED_BYTE, data);

	SOIL_free_image_data(data);
  }

  OpenGLTexture2D::~OpenGLTexture2D() {
	glDeleteTextures(1, &m_rendererID);
  }

  void OpenGLTexture2D::setData(void* data, uint32_t size) {

	uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
	PRIMAL_CORE_ASSERT(size == m_width * m_height * bpp, "Data must be entire texture!");
	glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);
  }

  void OpenGLTexture2D::bind(uint32_t slot) const {
	glBindTextureUnit(slot, m_rendererID);
  }
}
