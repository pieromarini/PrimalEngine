#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

namespace primal::renderer {

  class Texture {
	public:
	  Texture();
	  ~Texture();

	  void generate(uint32_t width, GLenum internalFormat, GLenum format, GLenum type, void* data);
	  void generate(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum type, void* data);
	  void generate(uint32_t width, uint32_t height, uint32_t depth, GLenum internalFormat, GLenum format, GLenum type, void* data);

	  void resize(uint32_t width = 0, uint32_t height = 0, uint32_t depth = 0);

	  void bind(int unit = -1);
	  void unbind();

	  void setWrapMode(GLenum wrapMode, bool doBind = false);
	  void setFilterMin(GLenum filter, bool doBind = false);
	  void setFilterMax(GLenum filter, bool doBind = false);

	  uint32_t m_id;
	  GLenum m_target = GL_TEXTURE_2D;
	  GLenum m_internalFormat = GL_RGBA;
	  GLenum m_format = GL_RGBA;
	  GLenum m_type = GL_UNSIGNED_BYTE;
	  GLenum m_filterMin = GL_LINEAR_MIPMAP_LINEAR;
	  GLenum m_filterMax = GL_LINEAR;
	  GLenum m_wrapS = GL_REPEAT;
	  GLenum m_wrapT = GL_REPEAT;
	  GLenum m_wrapR = GL_REPEAT;

	  bool m_mipmapping = true;
	  uint32_t m_width = 0;
	  uint32_t m_height = 0;
	  uint32_t m_depth = 0;
  };

}

#endif
