#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/log.h"
#include "renderer.h"
#include "rendererAPI.h"

#include "../../platform/openGL/openGLShader.h"

#include "shader.h"

namespace primal {

  ref_ptr<Shader> Shader::create(const std::string& filepath) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLShader>(filepath);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

  ref_ptr<Shader> Shader::create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
	switch (Renderer::getAPI()) {
	  case RendererAPI::API::None:    PRIMAL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	  case RendererAPI::API::OpenGL:  return createRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
	}

	PRIMAL_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
  }

  void ShaderLibrary::add(const std::string& name, const ref_ptr<Shader>& shader) {
	PRIMAL_CORE_ASSERT(!exists(name), "Shader already exists!");
	m_shaders[name] = shader;
  }

  void ShaderLibrary::add(const ref_ptr<Shader>& shader) {
	auto& name = shader->getName();
	add(name, shader);
  }

  ref_ptr<Shader> ShaderLibrary::load(const std::string& filepath) {
	auto shader = Shader::create(filepath);
	add(shader);
	return shader;
  }

  ref_ptr<Shader> ShaderLibrary::load(const std::string& name, const std::string& filepath) {
	auto shader = Shader::create(filepath);
	add(name, shader);
	return shader;
  }

  ref_ptr<Shader> ShaderLibrary::get(const std::string& name) {
	PRIMAL_CORE_ASSERT(exists(name), "Shader not found!");
	return m_shaders[name];
  }

  bool ShaderLibrary::exists(const std::string& name) const {
	return m_shaders.find(name) != m_shaders.end();
  }

}
