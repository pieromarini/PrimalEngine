#ifndef MATH_PLANE_H
#define MATH_PLANE_H

#include "core/math/linear_algebra/vector.h"

namespace primal::math {
  /*
	Plane as deducted by the generalized plane equation.
	Only defined in 3-dimensional space.
  */
  struct plane {
	vec3 Normal;
	float Distance;

	plane(const float a, const float b, const float c, const float d);
	plane(const vec3& normal, const vec3& point);
  };
}

#endif
