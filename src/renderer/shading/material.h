#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>

#include "shading_types.h"
#include "shader.h"
#include "texture.h"
#include "texture_cube.h"

#include "core/math/linear_algebra/vector.h"

namespace primal::renderer {

  enum MaterialType {
	MATERIAL_DEFAULT,
	MATERIAL_CUSTOM,
	MATERIAL_POST_PROCESS,
  };

  class Material {
	public:
	  Material();
	  Material(Shader* shader);

	  Shader* getShader();
	  void setShader(Shader* shader);

	  Material(const Material& rhs);

	  void setInt(std::string name, int value);
	  void setBool(std::string name, bool value);
	  void setFloat(std::string name, float value);

	  void setTexture(std::string name, Texture* value, uint32_t unit = 0);
	  void setTextureCube(std::string name, TextureCube* value, uint32_t unit = 0);

	  // void setVector(std::string name, math::Vector2 value);
	  void setVector(std::string name, math::vec3 value);
	  void setVector(std::string name, math::vec4 value);

	  // void setMatrix(std::string name, math::mat2 value);
	  void setMatrix(std::string name, math::mat3 value);
	  void setMatrix(std::string name, math::mat4 value);

	  std::map<std::string, UniformValue>* getUniforms();
	  std::map<std::string, UniformValueSampler>* getSamplerUniforms();

	  MaterialType type = MATERIAL_CUSTOM;
	  math::vec4 color{ 1.0f };

	  bool depthTest = true;
	  bool depthWrite = true;
	  GLenum depthCompare = GL_LESS;

	  bool cull = true;
	  GLenum cullFace =	  GL_BACK;
	  GLenum cullWindingOrder = GL_CCW;

	  bool blend = false;
	  GLenum blendSrc = GL_ONE;
	  GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;
	  GLenum blendEquation = GL_FUNC_ADD;

	  bool shadowCast = true;
	  bool shadowReceive = true;

	private:
	  Shader* m_shader;
	  std::map<std::string, UniformValue> m_uniforms;
	  std::map<std::string, UniformValueSampler> m_samplerUniforms;
  };

}

#endif
