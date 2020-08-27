#ifndef MATH_CONVERSIONS_H
#define MATH_CONVERSIONS_H

namespace primal::math {

  const float PI = 3.14159265359f;
  const float TAU = 6.28318530717f;

  inline float deg2Rad(float degrees) {
	return degrees / 180.0f * PI;
  }

  inline double deg2Rad(double degrees) {
	return degrees / 180.0 * PI;
  }

  inline float rad2Deg(float radians) {
	return radians / PI * 180.0f;
  }

  inline double rad2Deg(double radians) {
	return radians / PI * 180.0;
  }

}

#endif
