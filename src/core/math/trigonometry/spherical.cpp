#include <cmath>

#include "spherical.h"
#include "core/math/linear_algebra/operation.h"

namespace primal::math {

  spherical::spherical(const float Rho, const float Phi, const float Theta) : rho(Rho), phi(Phi), theta(Theta) {}

  spherical::spherical(const vec3& cartesian) {
	rho = length(cartesian);
	phi = atan2(length(cartesian.xy), cartesian.z);
	theta = atan2(cartesian.y, cartesian.x);
  }

  vec3 spherical::toCartesian() const {
	return vec3(
		rho * sin(phi) * cos(theta),
		rho * sin(phi) * sin(theta),
		rho * cos(phi));
  }

}
