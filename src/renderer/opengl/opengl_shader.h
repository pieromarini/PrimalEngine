#ifndef OPENGL_SHADER_H
#define OPENGL_SHADER_H

#include "renderer/shading/shader.h"

namespace primal::renderer::opengl {

  class OpenGLShader : public Shader {
	public:
	  OpenGLShader(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = {});

	  void load(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = {}) override;

	  void use() override;

	  bool hasUniform(std::string name) override;

	  void setInt(std::string location, int value) override;
	  void setBool(std::string location, bool value) override;
	  void setFloat(std::string location, float value) override;

	  // void setVector(std::string location, math::Vector2 value);
	  void setVector(std::string location, math::Vector3 value) override;
	  void setVector(std::string location, math::Vector4 value) override;

	  // void setVectorArray(std::string location, int size, std::vector<math::vec2>& values);
	  void setVectorArray(std::string location, std::size_t size, std::vector<math::Vector3>& values) override;
	  void setVectorArray(std::string location, std::size_t size, std::vector<math::Vector4>& values) override;

	  // void setMatrix(std::string location, math::mat2 value);
	  void setMatrix(std::string location, math::Matrix3 value) override;
	  void setMatrix(std::string location, math::Matrix4 value) override;
	private:
	  int getUniformLocation(std::string name);
  };

}

#endif
