#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "primal/core/core.h"

namespace primal {
  class Model {
	public:
	  Model(const std::string path) {
		loadModel(path);
	  }
	  void draw(Shader* shader);	
	private:
	  std::vector<Mesh> m_meshes;
	  std::vector<TextureWrapper> m_texturesLoaded;
	  std::string m_directory;

	  void loadModel(const std::string path);
	  void processNode(aiNode *node, const aiScene *scene);
	  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	  std::vector<TextureWrapper> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
  };
}
