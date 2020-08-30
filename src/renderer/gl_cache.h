#ifndef GL_CACHE_H
#define GL_CACHE_H

#include <glad/glad.h>

namespace primal::renderer {

  class GLCache {
	public:
	  GLCache();
	  ~GLCache();

	  void setDepthTest(bool enable);
	  void setDepthFunc(GLenum depthFunc);
	  void setBlend(bool enable);
	  void setBlendFunc(GLenum src, GLenum dst);
	  void setCull(bool enable);
	  void setCullFace(GLenum face);
	  void setPolygonMode(GLenum mode);

	  void switchShader(uint32_t ID);
	private:
	  bool m_depthTest{ false };
	  bool m_blend{ false };
	  bool m_cullFace{ false };

	  GLenum m_depthFunc{ 0 };
	  GLenum m_blendSrc{ 0 };
	  GLenum m_blendDst{ 0 };
	  GLenum m_frontFace{ 0 };
	  GLenum m_polygonMode{ 0 };

	  uint32_t m_activeShaderID{ 0 };
  };

}

#endif
