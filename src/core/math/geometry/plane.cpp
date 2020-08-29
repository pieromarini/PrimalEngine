#include "plane.h"

#include "../linear_algebra/operation.h"

namespace primal::math {

  plane::plane(const float a, const float b, const float c, const float d) {
	normal.x = a;
	normal.y = b;
	normal.z = c;
	distance = d;
  }

  plane::plane(const vec3& n, const vec3& point) {
	normal = n;
	distance = -dot(normal, point);
  }

}
