#ifndef MATH_SPHERICAL_H
#define MATH_SPHERICAL_H

#include "core/math/linear_algebra/vector.h"

namespace primal::math {

  struct spherical {
	float rho;   // distance coordinate from origin.
	float phi;   // counter-clockwise rotation from xy plane to z (z being up-axis in conventional math).
	float theta; // counter-clockwise rotation from x axis on xy plane.

	spherical(const float rho, const float phi, const float theta);

	spherical(const vec3& cartesian);

	vec3 ToCartesian() const;
  };

}

#endif
