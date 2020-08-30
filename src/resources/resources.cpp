#include "resources.h"

#include "tools/log.h"
#include "resources/shader_loader.h"
#include "resources/mesh_loader.h"
#include "resources/texture_loader.h"

namespace primal::renderer {

  std::map<StringId, Shader*> Resources::m_shaders{};
  std::map<StringId, Texture> Resources::m_textures{};
  std::map<StringId, TextureCube> Resources::m_texturesCube{};
  std::map<StringId, Entity*> Resources::m_meshes{};

  void Resources::init() {
	// TODO: init default resources.
  }

  void Resources::clean() {
	for (auto& mesh : m_meshes) {
	  delete mesh.second;
	}
	for (auto& shader : m_shaders) {
	  delete shader.second;
	}
  }

  Shader* Resources::loadShader(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines) {
	auto id = SID(name.data());
	if (m_shaders.find(id) != m_shaders.end()) {
	  return m_shaders[id];
	}

	auto shader = ShaderLoader::load(name, vsPath, fsPath, defines);
	m_shaders.insert({ id, shader });
	return m_shaders[id];
  }

  Shader* Resources::getShader(std::string name) {

	auto id = SID(name.data());
	if (m_shaders.find(id) != m_shaders.end()) {
	  return m_shaders[id];
	}

	PRIMAL_CORE_WARN("Shader resource {0} not found", name);
  }

  Texture* Resources::loadTexture(std::string name, std::string path, GLenum target, GLenum format, bool srgb) {
	auto id = SID(name.data());
	if (m_textures.find(id) != m_textures.end()) {
	  return &m_textures[id];
	}

	auto texture = TextureLoader::loadTexture(path, target, format, srgb);

	if (texture.m_width > 0) {
	  m_textures.insert({ id, texture });
	  return &m_textures[id];
	}

	return nullptr;
  }

  Texture* Resources::loadHDRTexture(std::string name, std::string path) {
	auto id = SID(name.data());
	if (m_textures.find(id) != m_textures.end()) {
	  return &m_textures[id];
	}

	auto texture = TextureLoader::loadHDRTexture(path);

	if (texture.m_width > 0) {
	  m_textures.insert({ id, texture });
	  return &m_textures[id];
	}

	return nullptr;
  }

  Texture* Resources::getTexture(std::string name) {
	auto id = SID(name.data());
	if (m_textures.find(id) != m_textures.end()) {
	  return &m_textures[id];
	}

	PRIMAL_CORE_WARN("Texture resource {0} not found", name);
  }

  TextureCube* Resources::loadTextureCube(std::string name, std::string folder) {
	auto id = SID(name.data());
	if (m_texturesCube.find(id) != m_texturesCube.end()) {
	  return &m_texturesCube[id];
	}

	auto texture = TextureLoader::loadTextureCube(folder);
	m_texturesCube.insert({ id, texture });
	return &m_texturesCube[id];
  }

  TextureCube* Resources::getTextureCube(std::string name) {
	auto id = SID(name.data());
	if (m_texturesCube.find(id) != m_texturesCube.end()) {
	  return &m_texturesCube[id];
	}

	PRIMAL_CORE_WARN("Texture cube resource {0} not found", name);
  }

  Entity* Resources::loadMesh(Renderer* renderer, std::string name, std::string path) {
	auto id = SID(name.data());
	if (m_meshes.find(id) != m_meshes.end()) {
	  return m_meshes[id];
	}

	auto mesh = MeshLoader::loadMesh(renderer, path);
	m_meshes.insert({ id, mesh });
	return m_meshes[id];
  }

  Entity* Resources::getMesh(std::string name) {
	auto id = SID(name.data());
	if (m_meshes.find(id) != m_meshes.end()) {
	  return m_meshes[id];
	}

	PRIMAL_CORE_WARN("Mesh resource {0} not found", name);
  }

}
