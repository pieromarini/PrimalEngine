#include <cmath>

#include "polar.h"
#include "core/math/linear_algebra/operation.h"

namespace primal::math {

  polar::polar(float r, float theta) {
	this->r = r;
	this->theta = theta;
  }

  polar::polar(vec2 cartesian) {
	r = length(cartesian);
	theta = atan2(cartesian.y, cartesian.x);
  }

  vec2 polar::ToCartesian() {
	return vec2(r * cos(theta), r * sin(theta));
  }

}
