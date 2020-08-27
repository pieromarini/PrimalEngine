#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include <vector>
#include <glad/glad.h>

#include "shading/texture.h"
#include "renderer.h"

namespace primal::renderer {

  class RenderTarget {
	public:
	  RenderTarget(unsigned int width, unsigned int height, GLenum type = GL_UNSIGNED_BYTE, unsigned int nrColorAttachments = 1, bool depthAndStencil = true);

	  Texture* getDepthStencilTexture();
	  Texture* getColorTexture(unsigned int index);

	  void resize(unsigned int width, unsigned int height);
	  void setTarget(GLenum target);

	  // TODO: this shouldn't be called by end-users, come up with something else than private/friend
	  //void Bind(bool clear = true, bool setViewport = true);

	  unsigned int m_id;

	  unsigned int m_width;
	  unsigned int m_height;
	  GLenum m_type;

	  bool HasDepthAndStencil;

	private:
	  GLenum m_Target = GL_TEXTURE_2D;
	  Texture m_DepthStencil;
	  std::vector<Texture> m_ColorAttachments;
	  friend Renderer;
  };

}

#endif
