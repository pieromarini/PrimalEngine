#include "material.h"
#include "renderer/shading_types.h"

namespace primal::renderer {

  Material::Material() {

  }

  Material::Material(Shader* shader) : m_shader{ shader } {

  }

  Shader* Material::getShader() {
	return m_shader;
  }

  void Material::setShader(Shader* shader) {
	m_shader = shader;
  }

  Material::Material(const Material& rhs) {
	m_shader = rhs.m_shader;
	type = rhs.type;
	color = rhs.color;
	depthTest = rhs.depthTest;
	depthWrite = rhs.depthWrite;
	depthCompare = rhs.depthCompare;

	cull = rhs.cull;
	cullFace = rhs.cullFace;
	cullWindingOrder = rhs.cullWindingOrder;

	blend = rhs.blend;
	blendSrc = rhs.blendSrc;
	blendDst = rhs.blendDst;
	blendEquation = rhs.blendEquation;

	m_uniforms = rhs.m_uniforms;
	m_samplerUniforms = rhs.m_samplerUniforms;
  }

  void Material::setInt(std::string name, int value) {
	m_uniforms[name].type = SHADER_TYPE_INT;
	m_uniforms[name].Int = value;
  }

  void Material::setBool(std::string name, bool value) {
	m_uniforms[name].type = SHADER_TYPE_BOOL;
	m_uniforms[name].Bool = value;
  }

  void Material::setFloat(std::string name, float value) {
	m_uniforms[name].type = SHADER_TYPE_FLOAT;
	m_uniforms[name].Float = value;
  }

  void Material::setTexture(std::string name, Texture* value, uint32_t unit) {
	m_samplerUniforms[name].unit = unit;
	m_samplerUniforms[name].texture = value;

	switch (value->m_target) {
	  case GL_TEXTURE_1D:
		m_samplerUniforms[name].type = SHADER_TYPE_SAMPLER1D;
		break;
	  case GL_TEXTURE_2D:
		m_samplerUniforms[name].type = SHADER_TYPE_SAMPLER2D;
		break;
	  case GL_TEXTURE_3D:
		m_samplerUniforms[name].type = SHADER_TYPE_SAMPLER3D;
		break;
	  case GL_TEXTURE_CUBE_MAP:
		m_samplerUniforms[name].type = SHADER_TYPE_SAMPLERCUBE;
		break;
	}

	if (m_shader) {
	  m_shader->use();
	  m_shader->setInt(name, unit);
	}
  }

  void Material::setTextureCube(std::string name, TextureCube* value, uint32_t unit) {
	m_samplerUniforms[name].unit = unit;
	m_samplerUniforms[name].type = SHADER_TYPE_SAMPLERCUBE;
	m_samplerUniforms[name].textureCube = value;

	if (m_shader) {
	  m_shader->use();
	  m_shader->setInt(name, unit);
	}
  }

  // void setVector(std::string name, math::Vector2 value);

  void Material::setVector(std::string name, math::Vector3 value) {
	m_uniforms[name].type = SHADER_TYPE_VEC3;
	m_uniforms[name].Vec3 = value;
  }
  void Material::setVector(std::string name, math::Vector4 value) {
	m_uniforms[name].type = SHADER_TYPE_VEC4;
	m_uniforms[name].Vec4 = value;
  }

  // void setMatrix(std::string name, math::mat2 value);

  void Material::setMatrix(std::string name, math::Matrix3 value) {
	m_uniforms[name].type = SHADER_TYPE_MAT3;
	m_uniforms[name].Mat3 = value;
  }
  void Material::setMatrix(std::string name, math::Matrix4 value) {
	m_uniforms[name].type = SHADER_TYPE_MAT4;
	m_uniforms[name].Mat4 = value;
  }

  std::map<std::string, UniformValue>* Material::getUniforms() {
	return &m_uniforms;
  }

  std::map<std::string, UniformValueSampler>* Material::getSamplerUniforms() {
	return &m_samplerUniforms;
  }

}
