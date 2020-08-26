#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <string>
#include <fstream>

#include "renderer/shading/shader.h"

namespace primal {
  
  using namespace primal::renderer;

  class ShaderLoader {
	public:
	  static Shader* load(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines = std::vector<std::string>());

	private:
	  static std::string readShader(std::ifstream& file, const std::string& name, std::string path);
  };

}

#endif
