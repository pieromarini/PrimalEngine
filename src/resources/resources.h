#include <map>
#include "util/sid.h"

#include "renderer/shading/shader.h"
#include "renderer/shading/texture.h"
#include "renderer/shading/texture_cube.h"
#include "mesh.h"

namespace primal {

  using namespace primal::renderer;

  class Renderer;

  class Resources {
	public:
	  static void init();
	  static void clean();

	  static Shader* loadShader(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines = {});
	  static Shader* getShader(std::string name);

	  static Texture* loadTexture(std::string name, std::string path, GLenum target = GL_TEXTURE_2D, GLenum format = GL_RGBA, bool srgb = false);
	  static Texture* loadHDRTexture(std::string name, std::string path);
	  static Texture* getTexture(std::string name);

	  static TextureCube* loadTextureCube(std::string name, std::string folder);
	  static TextureCube* getTextureCube(std::string name);

	  static Mesh* loadMesh(Renderer* renderer, std::string name, std::string path);
	  static Mesh* getMesh(std::string name);

	private:
	  Resources();
	  static std::map<StringId, Shader*> m_shaders;
	  static std::map<StringId, Texture> m_textures;
	  static std::map<StringId, TextureCube> m_texturesCube;
	  static std::map<StringId, Mesh*> m_meshes;
  };

}
