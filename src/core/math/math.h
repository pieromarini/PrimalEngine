#pragma once

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include "core/core.h"

namespace pm {

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

using mat3 = glm::mat3;
using mat4 = glm::mat4;

using quat = glm::quat;

struct Rect1D {
	// Constructor to always "sort" the min/max values
	Rect1D(u64 _min, u64 _max) : min(_min), max(_max) {
		if (max < min) {
			std::swap(min, max);
		}
	}

	u64 min;
	u64 max;
};

struct Rect1DF32 {
	// Constructor to always "sort" the min/max values
	Rect1DF32(f32 _min, f32 _max) : min(_min), max(_max) {
		if (max < min) {
			std::swap(min, max);
		}
	}

	f32 min;
	f32 max;
};

struct Rect2D {
	vec2 min;
	vec2 max;
};

struct Rect3D {
	vec3 min;
	vec3 max;
};

vec2 rect2DSize(Rect2D r);
vec2 rect2DCenter(Rect2D r);

Rect2D rect2DIntersect(Rect2D a, Rect2D b);
Rect2D rect2DPad(Rect2D r, f32 x);
Rect2D rect2DShift(Rect2D r, vec2 v);

f32 clamp1F32(Rect1DF32 r, f32 v);
b32 rect2DContains(Rect2D r, vec2 v);

vec4 mix(vec4 a, vec4 b, f32 t);

}// namespace pm
