#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include "core/math/matrix4.h"
#include "shading_types.h"

namespace primal::renderer {

  class Shader {
	public:
	  static Shader* create(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = {});

	  virtual void load(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = {}) = 0;

	  virtual void use() = 0;

	  virtual bool hasUniform(std::string name) = 0;

	  virtual void setInt(std::string location, int value) = 0;
	  virtual void setBool(std::string location, bool value) = 0;
	  virtual void setFloat(std::string location, float value) = 0;

	  // virtual void setVector(std::string location, math::Vector2 value) = 0;
	  virtual void setVector(std::string location, math::Vector3 value) = 0;
	  virtual void setVector(std::string location, math::Vector4 value) = 0;

	  // virtual void setVectorArray(std::string location, int size, const std::vector<math::vec2>& values) = 0;
	  virtual void setVectorArray(std::string location, std::size_t size, std::vector<math::Vector3>& values) = 0;
	  virtual void setVectorArray(std::string location, std::size_t size, std::vector<math::Vector4>& values) = 0;

	  // virtual void setMatrix(std::string location, math::mat2 value) = 0;
	  virtual void setMatrix(std::string location, math::Matrix3 value) = 0;
	  virtual void setMatrix(std::string location, math::Matrix4 value) = 0;

	  uint32_t m_id;
	  std::string m_name;
	  std::vector<Uniform> m_uniforms;
	  std::vector<VertexAttribute> m_attributes;
  };

};

#endif
