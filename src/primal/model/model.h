#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "primal/core/core.h"
#include "primal/renderer/texture.h"

namespace primal {
  class Model {
	public:
	  Model(const std::string path) {
		loadModel(path);
	  }

	  void draw(Shader* shader);	

	  // NOTE: Temporary. just to load textures for simple (1 mesh) models.
	  void loadTexture(const std::string path, const std::string type);

	private:
	  std::vector<Mesh> m_meshes;
	  std::vector<ref_ptr<Texture2D>> m_texturesLoaded;
	  std::string m_directory;

	  void loadModel(const std::string path);
	  void processNode(aiNode *node, const aiScene *scene);
	  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	  std::vector<ref_ptr<Texture2D>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
  };
}
