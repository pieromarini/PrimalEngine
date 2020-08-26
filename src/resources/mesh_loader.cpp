#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh_loader.h"
#include "components/mesh_component.h"
#include "scene/scene_manager.h"
#include "resources.h"
#include "renderer/shading/texture.h"
#include "tools/log.h"
#include "core/math/vector2.h"

#include "renderer/renderer.h"


namespace primal {

  std::vector<Mesh*> MeshLoader::m_meshStore = std::vector<Mesh*>();

  void MeshLoader::clean() {
	for (unsigned int i = 0; i < m_meshStore.size(); ++i) {
	  delete m_meshStore[i];
	}
  }

  Entity* MeshLoader::loadMesh(Renderer* renderer, std::string path, bool setDefaultMaterial) {
	PRIMAL_CORE_INFO("Loading mesh file at: {0}", path);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
	  PRIMAL_CORE_ERROR("Assimp failed to load model at path: {0}", path);
	  return nullptr;
	}

	std::string directory = path.substr(0, path.find_last_of("/"));

	PRIMAL_CORE_INFO("Succesfully loaded: {0}", path);

	return MeshLoader::processNode(renderer, scene->mRootNode, scene, directory, setDefaultMaterial);
  }

  Entity* MeshLoader::processNode(Renderer* renderer, aiNode* aNode, const aiScene* aScene, std::string directory, bool setDefaultMaterial) {
	auto entity = Entity::create("MeshRoot", nullptr);

	for (uint32_t i = 0; i < aNode->mNumMeshes; ++i) {
	  math::Vector3 boxMin, boxMax;
	  aiMesh* assimpMesh = aScene->mMeshes[aNode->mMeshes[i]];
	  aiMaterial* assimpMat = aScene->mMaterials[assimpMesh->mMaterialIndex];

	  auto mesh = parseMesh(assimpMesh, aScene, boxMin, boxMax);
	  Material* material = nullptr;

	  if (setDefaultMaterial) {
		material = parseMaterial(renderer, assimpMat, aScene, directory);
	  }

	  if (aNode->mNumMeshes == 1) {
		if (setDefaultMaterial)
		  entity->addComponent<MeshComponent>(mesh, material, boxMin, boxMax);
		else
		  entity->addComponent<MeshComponent>(mesh, boxMin, boxMax);
	  } else {
		auto child = Entity::create("name", nullptr);
		child->addComponent<MeshComponent>(mesh, material, boxMin, boxMax);
		entity->transform->addChild(child->transform);
	  }
	}

	for (unsigned int i = 0; i < aNode->mNumChildren; ++i) {
	  auto childEntity = MeshLoader::processNode(renderer, aNode->mChildren[i], aScene, directory, setDefaultMaterial);
	  entity->transform->addChild(childEntity->transform);
	}

	return entity;
  }

  Mesh* MeshLoader::parseMesh(aiMesh* aMesh, const aiScene* aScene, math::Vector3& out_Min, math::Vector3& out_Max) {
	std::vector<math::Vector3> positions;
	std::vector<math::Vector2> uv;
	std::vector<math::Vector3> normals;
	std::vector<math::Vector3> tangents;
	std::vector<math::Vector3> bitangents;
	std::vector<unsigned int> indices;

	positions.resize(aMesh->mNumVertices);
	normals.resize(aMesh->mNumVertices);
	if (aMesh->mNumUVComponents > 0) {
	  uv.resize(aMesh->mNumVertices);
	  tangents.resize(aMesh->mNumVertices);
	  bitangents.resize(aMesh->mNumVertices);
	}
	// we assume a constant of 3 vertex indices per face as we always triangulate in Assimp's
	// post-processing step; otherwise you'll want transform this to a more  flexible scheme.
	indices.resize(aMesh->mNumFaces * 3);

	// store min/max point in local coordinates for calculating approximate bounding box.
	math::Vector3 pMin(99999.0);
	math::Vector3 pMax(-99999.0);

	for (unsigned int i = 0; i < aMesh->mNumVertices; ++i) {
	  positions[i] = math::Vector3(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z);
	  normals[i] = math::Vector3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z);
	  if (aMesh->mTextureCoords[0]) {
		uv[i] = math::Vector2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y);
	  }
	  if (aMesh->mTangents) {
		tangents[i] = math::Vector3(aMesh->mTangents[i].x, aMesh->mTangents[i].y, aMesh->mTangents[i].z);
		bitangents[i] = math::Vector3(aMesh->mBitangents[i].x, aMesh->mBitangents[i].y, aMesh->mBitangents[i].z);
	  }
	  if (positions[i].x < pMin.x) pMin.x = positions[i].x;
	  if (positions[i].y < pMin.y) pMin.y = positions[i].y;
	  if (positions[i].z < pMin.z) pMin.z = positions[i].z;
	  if (positions[i].x > pMax.x) pMax.x = positions[i].x;
	  if (positions[i].y > pMax.y) pMax.y = positions[i].y;
	  if (positions[i].z > pMax.z) pMax.z = positions[i].z;
	}
	for (unsigned int f = 0; f < aMesh->mNumFaces; ++f) {
	  for (unsigned int i = 0; i < 3; ++i) {
		indices[f * 3 + i] = aMesh->mFaces[f].mIndices[i];
	  }
	}

	Mesh* mesh = new Mesh();
	mesh->m_positions = positions;
	mesh->m_uv = uv;
	mesh->m_normals = normals;
	mesh->m_tangents = tangents;
	mesh->m_bitangents = bitangents;
	mesh->m_indices = indices;
	mesh->topology = TRIANGLES;
	mesh->finalize(true);

	out_Min.x = pMin.x;
	out_Min.y = pMin.y;
	out_Min.z = pMin.z;
	out_Max.x = pMax.x;
	out_Max.y = pMax.y;
	out_Max.z = pMax.z;

	MeshLoader::m_meshStore.push_back(mesh);

	return mesh;
  }

  Material* MeshLoader::parseMaterial(Renderer* renderer, aiMaterial* aMaterial, const aiScene* aScene, std::string directory) {
	// create a unique default material for each loaded mesh.
	Material* material;
	// check if diffuse texture has alpha, if so: make alpha blend material;
	aiString file;
	aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
	std::string diffPath = std::string(file.C_Str());
	bool alpha = false;
	if (diffPath.find("_alpha") != std::string::npos) {
	  material = renderer->createMaterial("alpha discard");
	  alpha = true;
	} else {
	  material = renderer->createMaterial();
	}

	/* NOTE:
	   About texture types:
	   We use a PBR metallic/roughness workflow so the loaded models are expected to have 
	   textures conform the workflow: albedo, (normal), metallic, roughness, (ao). Since Assimp
	   made certain assumptions regarding possible types of loaded textures it doesn't directly
	   translate to our model thus we make some assumptions as well which the 3D author has to
	   comply with if he wants the mesh(es) to directly render with its specified textures:

	   - aiTextureType_DIFFUSE:   Albedo
	   - aiTextureType_NORMALS:   Normal
	   - aiTextureType_SPECULAR:  metallic
	   - aiTextureType_SHININESS: roughness 
	   - aiTextureType_AMBIENT:   AO (ambient occlusion)
	   - aiTextureType_EMISSIVE:  Emissive
	   */
	if (aMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
	  // we only load the first of the list of diffuse textures, we don't really care about
	  // meshes with multiple diffuse layers; same holds for other texture types.
	  aiString file;
	  aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
	  std::string fileName = MeshLoader::processPath(&file, directory);
	  // we name the texture the same as the filename as to reduce naming conflicts while still only loading unique textures.
	  Texture* texture = Resources::loadTexture(fileName, fileName, GL_TEXTURE_2D, alpha ? GL_RGBA : GL_RGB, true);
	  if (texture) {
		material->setTexture("TexAlbedo", texture, 3);
	  }
	}
	if (aMaterial->GetTextureCount(aiTextureType_DISPLACEMENT) > 0) {
	  aiString file;
	  aMaterial->GetTexture(aiTextureType_DISPLACEMENT, 0, &file);
	  std::string fileName = MeshLoader::processPath(&file, directory);

	  Texture* texture = Resources::loadTexture(fileName, fileName);
	  if (texture) {
		material->setTexture("TexNormal", texture, 4);
	  }
	}
	if (aMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
	  aiString file;
	  aMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
	  std::string fileName = MeshLoader::processPath(&file, directory);

	  Texture* texture = Resources::loadTexture(fileName, fileName);
	  if (texture) {
		material->setTexture("TexMetallic", texture, 5);
	  }
	}
	if (aMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
	  aiString file;
	  aMaterial->GetTexture(aiTextureType_SHININESS, 0, &file);
	  std::string fileName = MeshLoader::processPath(&file, directory);

	  Texture* texture = Resources::loadTexture(fileName, fileName);
	  if (texture) {
		material->setTexture("TexRoughness", texture, 6);
	  }
	}
	if (aMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0) {
	  aiString file;
	  aMaterial->GetTexture(aiTextureType_AMBIENT, 0, &file);
	  std::string fileName = MeshLoader::processPath(&file, directory);

	  Texture* texture = Resources::loadTexture(fileName, fileName);
	  if (texture) {
		material->setTexture("TexAO", texture, 7);
	  }
	}

	return material;
  }

  std::string MeshLoader::processPath(aiString* aPath, std::string directory) {
	std::string path = std::string(aPath->C_Str());
	// parse path directly if path contains "/" indicating it is an absolute path;  otherwise
	// parse as relative.
	if (path.find(":/") == std::string::npos || path.find(":\\") == std::string::npos)
	  path = directory + "/" + path;
	return path;
  }
}
