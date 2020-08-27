#include <string>
#include <vector>

#include "core/math/vector3.h"
#include "renderer/shading/material.h"
#include "resources/mesh.h"
#include "scene/entity.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiString;

namespace primal::renderer {

  class Renderer;

  class MeshLoader {
	public:
	  static void clean();
	  static Entity* loadMesh(Renderer* renderer, std::string path, bool setDefaultMaterial = true);

	private:
	  static Entity* processNode(Renderer* renderer, aiNode* aNode, const aiScene* aScene, std::string directory, bool setDefaultMaterial);
	  static Mesh* parseMesh(aiMesh* aMesh, const aiScene* aScene, math::Vector3& out_Min, math::Vector3& out_Max);
	  static Material* parseMaterial(Renderer* renderer, aiMaterial* aMaterial, const aiScene* aScene, std::string directory);

	  static std::string processPath(aiString* path, std::string directory);

	  static std::vector<Mesh*> m_meshStore;

  };

}
