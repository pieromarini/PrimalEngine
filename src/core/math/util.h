#ifndef UTIL_H
#define UTIL_H

#include <initializer_list>

namespace primal::math {
  class Util {
	public:
	  static const float PI;
	  static const float PI_HALF;
	  static const float EPSILON;
	  static const float DEG2RAD;
	  static const float RAD2DEG;

	  static float abs(float number);
	  static int abs(int number);
	  static float acos(float number);
	  static float asin(float number);
	  static float atan(float number);
	  static float atan2(float y, float x);
	  static float ceil(float number);
	  static int ceilToInt(float number);
	  static float clamp(float start, float end, float number);
	  static int clamp(int start, int end, int number);
	  static float clamp01(float number);
	  static int closestPowerOfTwo(int number);
	  static float cos(float radian);
	  static float exp(float power);
	  static float floor(float number);
	  static int floorToInt(float number);
	  static float inverseLerp(float start, float end, float number);
	  static bool isPowerOfTwo(int number);
	  static float lerpUnclamped(float start, float end, float time);
	  static float lerp(float start, float end, float time);
	  static float ln(float number);
	  static float log(float number, float base);
	  static float log10(float number);
	  static float max(float a, float b);
	  static float max(std::initializer_list<float> numbers);
	  static int max(std::initializer_list<int> numbers);
	  static float min(float a, float b);
	  static float min(std::initializer_list<float> numbers);
	  static int min(std::initializer_list<int> numbers);
	  static float moveTowards(float current, float target, float maxDelta);
	  static int nextPowerOfTwo(int number);
	  static float pow(float number, float power);
	  static float repeat(float t, float length);
	  static float round(float number);
	  static int roundToInt(float number);
	  static int sign(float number);
	  static float sin(float radian);
	  static float smoothStep(float start, float end, float number);
	  static float sqrt(float number);
	  static float square(float number);
	  static int square(int number);
	  static float tan(float radian);

	  static bool fuzzyEquals(float, float);
  };
}

#endif
