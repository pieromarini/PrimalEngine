#pragma once

#include "primal/core/core.h"
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace primal {

  class Shader {
	public:
	  virtual ~Shader() = default;

	  virtual void bind() const = 0;
	  virtual void unbind() const = 0;

	  virtual void setInt(const std::string& name, int value) = 0;
	  virtual void setFloat(const std::string& name, const float value) = 0;
	  virtual void setFloat3(const std::string& name, const glm::vec3& value) = 0;
	  virtual void setFloat4(const std::string& name, const glm::vec4& value) = 0;
	  virtual void setMat4(const std::string& name, const glm::mat4& value) = 0;

	  virtual const std::string& getName() const = 0;

	  static ref_ptr<Shader> create(const std::string& filepath);
	  static ref_ptr<Shader> create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
  };

  class ShaderLibrary {
	public:
	  void add(const std::string& name, const ref_ptr<Shader>& shader);
	  void add(const ref_ptr<Shader>& shader);
	  ref_ptr<Shader> load(const std::string& filepath);
	  ref_ptr<Shader> load(const std::string& name, const std::string& filepath);

	  ref_ptr<Shader> get(const std::string& name);

	  bool exists(const std::string& name) const;
	private:
	  std::unordered_map<std::string, ref_ptr<Shader>> m_shaders;
  };

}
