#include "model.h"
#include "assimp/material.h"
#include "primal/core/log.h"
#include "primal/renderer/texture.h"

namespace primal {

  void Model::draw(Shader* shader) {
	for(auto &mesh : m_meshes)
	  mesh.draw(shader);
  }

  void Model::loadTexture(const std::string& path, const std::string& type) {
	auto texture = Texture2D::create(path, type);

	for (auto& mesh : m_meshes) {
	  mesh.setTexture(texture);
	}
  }

  void Model::loadModel(const std::string path) {
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
	  PRIMAL_CORE_ERROR("[ASSIMP] {0}", import.GetErrorString());
	  return;
	}

	m_directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
  }  

  // NOTE: Resursively processes all meshes
  void Model::processNode(aiNode *node, const aiScene *scene) {
	for(unsigned int i = 0; i < node->mNumMeshes; ++i) {
	  auto mesh = scene->mMeshes[node->mMeshes[i]]; 
	  m_meshes.push_back(processMesh(mesh, scene));			
	}

	for(unsigned int i = 0; i < node->mNumChildren; ++i) {
	  processNode(node->mChildren[i], scene);
	}
  }



  Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	ref_ptr<Material> material;
	// Walk through each of the mesh's vertices
	for(unsigned int i = 0; i < mesh->mNumVertices; ++i) {
	  Vertex vertex{};
	  glm::vec3 vec;

	  // positions
	  vec.x = mesh->mVertices[i].x;
	  vec.y = mesh->mVertices[i].y;
	  vec.z = mesh->mVertices[i].z;
	  vertex.Position = vec;

	  // normals
	  vec.x = mesh->mNormals[i].x;
	  vec.y = mesh->mNormals[i].y;
	  vec.z = mesh->mNormals[i].z;
	  vertex.Normal = vec;

	  // texture coordinates
	  // NOTE: Only handles 1 set of texture coordinates
	  if(mesh->mTextureCoords[0]) {
		glm::vec2 vec;
		vec.x = mesh->mTextureCoords[0][i].x; 
		vec.y = mesh->mTextureCoords[0][i].y;
		vertex.TexCoords = vec;
	  } else {
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
	  }

	  // tangents and bitangents
	  if (mesh->HasTangentsAndBitangents()) {
		vec.x = mesh->mTangents[i].x;
		vec.y = mesh->mTangents[i].y;
		vec.z = mesh->mTangents[i].z;
		vertex.Tangent = vec;

		vec.x = mesh->mBitangents[i].x;
		vec.y = mesh->mBitangents[i].y;
		vec.z = mesh->mBitangents[i].z;
		vertex.Bitangent = vec;
	  }

	  vertices.push_back(vertex);
	}

	for(std::size_t i = 0; i < mesh->mNumFaces; ++i) {
	  aiFace face = mesh->mFaces[i];
	  for(std::size_t j = 0; j < face.mNumIndices; ++j)
		indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0) {
	  aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];    

	  // NOTE: rvalue created moves into Material instance?
	  material = Material::create(loadMaterialTextures(aiMat));
	}


	return Mesh(vertices, indices, material);
  }

  std::vector<ref_ptr<Texture2D>> Model::loadMaterialTextures(aiMaterial *mat) {
	std::vector<ref_ptr<Texture2D>> textures;

	/* NOTE: PrimalEngine assumes a convention for sampler names in the shaders.
	 * Each diffuse texture should be named as 'texture_diffuseN' where N is a
	 * sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	 * Same applies to other textures as the following list summarizes:
	 * diffuse: texture_diffuseN
	 * specular: texture_specularN
	 * normal: texture_normalN
	 * height: texture_heightN
	 */
	std::array<std::pair<aiTextureType, std::string>, 4> types {{
	  { aiTextureType_DIFFUSE, "texture_diffuse" }, { aiTextureType_SPECULAR, "texture_specular" },
	  { aiTextureType_HEIGHT, "texture_height" }, { aiTextureType_NORMALS, "texture_normal" }
	}};

	std::string fullPath;

	for (const auto& [type, typeName] : types) {
	  for(std::size_t i = 0; i < mat->GetTextureCount(type); ++i) {
		aiString texturePath;
		mat->GetTexture(type, i, &texturePath);
		fullPath = m_directory + "/" + texturePath.C_Str();

		textures.emplace_back(Texture2D::create(fullPath, typeName));
	  }
	}

	return textures;
  }
}
