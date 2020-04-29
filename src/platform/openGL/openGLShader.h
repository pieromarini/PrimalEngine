#pragma once

#include "primal/renderer/shader.h"
#include "primal/core/log.h"
#include "primal/core/core.h"
#include <glm/glm.hpp>

using GLenum = unsigned int;

namespace primal {

  class OpenGLShader : public Shader {
	public:
	  OpenGLShader(const std::string& filepath);
	  OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	  ~OpenGLShader() override;

	  void bind() const override;
	  void unbind() const override;

	  void setInt(const std::string& name, int value) override;
	  void setFloat(const std::string& name, const float value) override;
	  void setFloat3(const std::string& name, const glm::vec3& value) override;
	  void setFloat4(const std::string& name, const glm::vec4& value) override;
	  void setMat4(const std::string& name, const glm::mat4& value) override;

	  const std::string& getName() const override { return m_name; }
	private:
	  std::string readFile(const std::string& filepath);
	  std::unordered_map<GLenum, std::string> preProcess(const std::string& source);
	  void compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	  void uploadUniformInt(const std::string& name, int value);

	  void uploadUniformFloat(const std::string& name, float value);
	  void uploadUniformFloat2(const std::string& name, const glm::vec2& value);
	  void uploadUniformFloat3(const std::string& name, const glm::vec3& value);
	  void uploadUniformFloat4(const std::string& name, const glm::vec4& value);

	  void uploadUniformMat3(const std::string& name, const glm::mat3& matrix);
	  void uploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	  uint32_t m_rendererID;
	  std::string m_name;
  };

}
