#pragma once

#include "primal/renderer/texture.h"

#include <glad/glad.h>

namespace primal {

  class OpenGLTexture2D : public Texture2D {
	public:
	  OpenGLTexture2D(uint32_t width, uint32_t height);
	  OpenGLTexture2D(const std::string& path, const std::string& type);
	  ~OpenGLTexture2D() override;

	  uint32_t getWidth() const override { return m_width;  }
	  uint32_t getHeight() const override { return m_height; }

	  void setData(void* data, uint32_t size) override;

	  void bind(uint32_t slot = 0) const override;
	private:
	  uint32_t m_width, m_height;
	  GLenum m_internalFormat, m_dataFormat;
  };

}
