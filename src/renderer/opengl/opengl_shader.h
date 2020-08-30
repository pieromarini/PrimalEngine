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

	  void setVector(std::string location, math::vec2 value) override;
	  void setVector(std::string location, math::vec3 value) override;
	  void setVector(std::string location, math::vec4 value) override;

	  void setVectorArray(std::string location, std::size_t size, std::vector<math::vec2>& values) override;
	  void setVectorArray(std::string location, std::size_t size, std::vector<math::vec3>& values) override;
	  void setVectorArray(std::string location, std::size_t size, std::vector<math::vec4>& values) override;

	  void setMatrix(std::string location, math::mat2 value) override;
	  void setMatrix(std::string location, math::mat3 value) override;
	  void setMatrix(std::string location, math::mat4 value) override;
	private:
	  int getUniformLocation(std::string name);
  };

}

#endif
