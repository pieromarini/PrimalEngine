#include "gl_cache.h"

namespace primal::renderer {

  GLCache::GLCache() {
  }

  GLCache::~GLCache() {
  }

  void GLCache::setDepthTest(bool enable) {
	if (m_depthTest != enable) {
	  m_depthTest = enable;
	  if (enable)
		glEnable(GL_DEPTH_TEST);
	  else
		glDisable(GL_DEPTH_TEST);
	}
  }

  void GLCache::setDepthFunc(GLenum depthFunc) {
	if (m_depthFunc != depthFunc) {
	  m_depthFunc = depthFunc;
	  glDepthFunc(depthFunc);
	}
  }

  void GLCache::setBlend(bool enable) {
	if (m_blend != enable) {
	  m_blend = enable;
	  if (enable)
		glEnable(GL_BLEND);
	  else
		glDisable(GL_BLEND);
	}
  }

  void GLCache::setBlendFunc(GLenum src, GLenum dst) {
	if (m_blendSrc != src || m_blendDst != dst) {
	  m_blendSrc = src;
	  m_blendDst = dst;
	  glBlendFunc(src, dst);
	}
  }

  void GLCache::setCull(bool enable) {
	if (m_cullFace != enable) {
	  m_cullFace = enable;
	  if (enable)
		glEnable(GL_CULL_FACE);
	  else
		glDisable(GL_CULL_FACE);
	}
  }

  void GLCache::setCullFace(GLenum face) {
	if (m_frontFace != face) {
	  m_frontFace = face;
	  glCullFace(face);
	}
  }

  void GLCache::setPolygonMode(GLenum mode) {
	if (m_polygonMode != mode) {
	  m_polygonMode = mode;
	  glPolygonMode(GL_FRONT_AND_BACK, mode);
	}
  }

  void GLCache::switchShader(uint32_t ID) {
	if (m_activeShaderID != ID) {
	  glUseProgram(ID);
	}
  }
}
