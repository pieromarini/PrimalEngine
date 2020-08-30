#include <glad/glad.h>

#include <string>

#include "renderer/shading/texture.h"
#include "renderer/shading/texture_cube.h"

namespace primal {

  using namespace primal::renderer;

  class TextureLoader {
	public:
	  static Texture loadTexture(std::string path, GLenum target, GLenum internalFormat, bool srgb = false);
	  static Texture loadHDRTexture(std::string path);

	  static TextureCube loadTextureCube(std::string top, std::string bottom, std::string left, std::string right, std::string front, std::string back);
	  static TextureCube loadTextureCube(std::string folder);
  };

}
