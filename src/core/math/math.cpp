#include "core/math/math.h"
#include "glm/common.hpp"

namespace pm {

vec2 rect2DSize(Rect2D r) {
	return glm::abs(r.max - r.min);
}

vec2 rect2DCenter(Rect2D r) {
	return (r.max + r.min) / 2.0f;
}

Rect2D rect2DIntersect(Rect2D a, Rect2D b) {
	return Rect2D{
		.min = vec2(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)),
		.max = vec2(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y))
	};
}

Rect2D rect2DPad(Rect2D r, f32 x) {
	return { .min = r.min - vec2{ x, x }, .max = r.max + vec2{ x, x } };
}

Rect2D rect2DShift(Rect2D r, vec2 v) {
	return { .min = r.min + v, .max = r.max + v };
}

f32 clamp1F32(Rect1DF32 r, f32 v) {
	v = Clamp(r.min, v, r.max);
	return v;
}

b32 rect2DContains(Rect2D r, vec2 v) {
	return (r.min.x <= v.x && v.x <= r.max.x) && (r.min.y <= v.y && v.y <= r.max.y);
}

};// namespace pm
