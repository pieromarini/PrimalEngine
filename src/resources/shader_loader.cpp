#include "shader_loader.h"
#include "tools/log.h"

namespace primal {

  Shader* ShaderLoader::load(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines) {
	std::ifstream vsFile, fsFile;
	vsFile.open(vsPath);
	fsFile.open(fsPath);

	if (!vsFile.is_open() || !fsFile.is_open()) {
	  PRIMAL_CORE_ERROR("Shader failed to load at path: {0} and {1}", vsPath, fsPath);
	  return nullptr;
	}

	std::string vsDirectory = vsPath.substr(0, vsPath.find_last_of("/\\"));
	std::string fsDirectory = fsPath.substr(0, fsPath.find_last_of("/\\"));
	std::string vsSource = readShader(vsFile, name, vsPath);
	std::string fsSource = readShader(fsFile, name, fsPath);

	auto shader = Shader::create(name, vsPath, fsPath, defines);

	vsFile.close();
	fsFile.close();

	return shader;
  }

  std::string ShaderLoader::readShader(std::ifstream& file, const std::string& name, std::string path) {
	std::string directory = path.substr(0, path.find_last_of("/\\"));
	std::string source, line;
	while (std::getline(file, line)) {

	  if (line.substr(0, 8) == "#include") {
		std::string includePath = directory + "/" + line.substr(9);
		std::ifstream includeFile(includePath);
		if (includeFile.is_open()) {

		  source += readShader(includeFile, name, includePath);
		} else {
		  PRIMAL_CORE_ERROR("Shader {0}: include: {1} failed to open", name, includePath);
		}
		includeFile.close();
	  } else
		source += line + "\n";
	}
	return source;
  }

}
