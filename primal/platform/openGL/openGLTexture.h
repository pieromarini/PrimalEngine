#pragma once

#include "../../primal/renderer/texture.h"

#include <glad/gl.h>

namespace primal {

  class OpenGLTexture2D : public Texture2D {
	public:
	  OpenGLTexture2D(uint32_t width, uint32_t height);
	  OpenGLTexture2D(const std::string& path);
	  virtual ~OpenGLTexture2D();

	  virtual uint32_t getWidth() const override { return m_width;  }
	  virtual uint32_t getHeight() const override { return m_height; }

	  virtual void setData(void* data, uint32_t size) override;

	  virtual void bind(uint32_t slot = 0) const override;
	private:
	  std::string m_path;
	  uint32_t m_width, m_height;
	  uint32_t m_rendererID;
	  GLenum m_internalFormat, m_dataFormat;
  };

}
