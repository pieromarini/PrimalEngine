#ifndef TEXTURE_CUBE_H
#define TEXTURE_CUBE_H

#include <glad/glad.h>

#include "texture.h"

namespace primal::renderer {

  class TextureCube {
	public:
	  TextureCube();
	  ~TextureCube();

	  void defaultInitialize(uint32_t width, int32_t height, GLenum format, GLenum type, bool mipmap = false);

	  void generateFace(GLenum face, uint32_t width, uint32_t height, GLenum format, GLenum type, unsigned char* data);

	  void setMipFace(GLenum face, uint32_t width, uint32_t height, GLenum format, GLenum type, uint32_t mipLevel, unsigned char* data);

	  void resize(uint32_t width, uint32_t height);

	  void bind(int unit = -1);
	  void unbind();

	  uint32_t m_id;
	  GLenum m_internalFormat = GL_RGBA;
	  GLenum m_format = GL_RGBA;
	  GLenum m_type = GL_UNSIGNED_BYTE;
	  GLenum m_filterMin = GL_LINEAR;
	  GLenum m_filterMax = GL_LINEAR;
	  GLenum m_wrapS = GL_CLAMP_TO_EDGE;
	  GLenum m_wrapT = GL_CLAMP_TO_EDGE;
	  GLenum m_wrapR = GL_CLAMP_TO_EDGE;
	  bool m_mipmapping = false;

	  uint32_t faceWidth = 0;
	  uint32_t faceHeight = 0;
  };

}

#endif
