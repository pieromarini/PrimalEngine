#ifndef MATH_POLAR_H
#define MATH_POLAR_H

#include "core/math/linear_algebra/vector.h"

namespace primal::math {

  struct polar {
	float r;
	float theta;

	polar(float r, float theta);
	polar(vec2 cartesian);

	vec2 ToCartesian();
  };

}

#endif
