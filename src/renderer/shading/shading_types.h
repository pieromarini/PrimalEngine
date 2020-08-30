#ifndef SHADING_TYPES_H
#define SHADING_TYPES_H

#include <string>

#include "core/math/linear_algebra/matrix.h"
#include "core/math/linear_algebra/vector.h"

namespace primal::renderer {

  class Texture;
  class TextureCube;

  enum SHADER_TYPE {
	SHADER_TYPE_BOOL,
	SHADER_TYPE_INT,
	SHADER_TYPE_FLOAT,
	SHADER_TYPE_SAMPLER1D,
	SHADER_TYPE_SAMPLER2D,
	SHADER_TYPE_SAMPLER3D,
	SHADER_TYPE_SAMPLERCUBE,
	SHADER_TYPE_VEC2,
	SHADER_TYPE_VEC3,
	SHADER_TYPE_VEC4,
	SHADER_TYPE_MAT2,
	SHADER_TYPE_MAT3,
	SHADER_TYPE_MAT4
  };

  struct Uniform {
	SHADER_TYPE type;
	std::string name;
	int size;
	uint32_t location;
  };

  struct UniformValue {
	SHADER_TYPE type;

	union {
	  // TODO: Add missing types (vec2, mat2).
	  bool Bool;
	  int Int;
	  float Float;
	  math::vec2 Vec2;
	  math::vec3 Vec3;
	  math::vec4 Vec4;
	  math::mat2 Mat2;
	  math::mat3 Mat3;
	  math::mat4 Mat4;
	};

	UniformValue() = default;
  };

  struct UniformValueSampler {
	SHADER_TYPE type;
	uint32_t unit;

	union {
	  Texture* texture;
	  TextureCube* textureCube;
	};

	UniformValueSampler() = default;
  };

  struct VertexAttribute {
	SHADER_TYPE type;
	std::string name;
	int size;
	uint32_t location;
  };

}

#endif
