#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iostream>

#include "util.h"

namespace primal::math {

  const float Util::PI = static_cast<float>(M_PI);
  const float Util::PI_HALF = 0.5f * PI;
  const float Util::EPSILON = std::numeric_limits<float>::epsilon();
  const float Util::DEG2RAD = Util::PI / 180.f;
  const float Util::RAD2DEG = 180.f * static_cast<float>(M_1_PI);
  const float Util::TAU = static_cast<float>(M_PI * 2);

  float Util::abs(float number) { return number < 0 ? -number : number; }

  int Util::abs(int number) { return number < 0 ? -number : number; }

  float Util::acos(float number) { return acosf(number); }
  float Util::asin(float number) { return asinf(number); }
  float Util::atan(float number) { return atanf(number); }
  float Util::atan2(float y, float x) { return atan2f(y, x); }
  float Util::ceil(float number) { return ceilf(number); }
  int Util::ceilToInt(float number) { return static_cast<int>(ceilf(number)); }
  float Util::clamp(float start, float end, float number) {
	return number < start ? start : (number > end ? end : number);
  }
  int Util::clamp(int start, int end, int number) {
	return number < start ? start : (number > end ? end : number);
  }
  float Util::clamp01(float number) { return Util::clamp(0.f, 1.f, number); }
  int Util::closestPowerOfTwo(int number) {
	if (number < 0)
	  throw std::out_of_range{
		"Utility::ClosestPowerOfTwo => Negative numbers are not supported."};
	int ceil = number - 1;
	for (std::size_t i = 1; i < sizeof(ceil) * std::numeric_limits<unsigned char>::digits; i *= 2) {
	  ceil |= ceil >> i;
	}
	++ceil;
	int floor = ceil >> 1;
	if (abs(number - ceil) < abs(number - floor)) {
	  return ceil;
	} else {
	  return floor;
	}
  }
  float Util::cos(float radian) { return cosf(radian); }
  float Util::exp(float power) { return expf(power); }
  float Util::floor(float number) { return floorf(number); }
  int Util::floorToInt(float number) { return static_cast<int>(floorf(number)); }
  float Util::inverseLerp(float start, float end, float number) {
	return (number - start) / (end - start);
  }
  bool Util::isPowerOfTwo(int number) {
	if (number < 0)
	  throw std::out_of_range{
		"Utility::IsPowerOfTwo => Negative numbers are not supported."};
	return !(number == 0) && !(number & (number - 1));
  }
  float Util::lerpUnclamped(float start, float end, float time) {
	return start * (1 - time) + time * end;
  }
  float Util::lerp(float start, float end, float time) {
	time = clamp01(time);
	return start * (1 - time) + time * end;
  }
  float Util::ln(float number) { return logf(number); }
  float Util::log(float number, float base) { return logf(number) / std::log(base); }
  float Util::log10(float number) { return log10f(number); }
  float Util::max(float a, float b) { return std::max(a, b); }
  float Util::max(std::initializer_list<float> numbers) {
	return std::max(numbers);
  }
  int Util::max(std::initializer_list<int> numbers) { return std::max(numbers); }
  float Util::min(float a, float b) { return std::min(a, b); }
  float Util::min(std::initializer_list<float> numbers) {
	return std::min(numbers);
  }
  int Util::min(std::initializer_list<int> numbers) { return std::min(numbers); }

  float Util::moveTowards(float current, float target, float maxDelta) {
	return std::min(current + maxDelta, target);
  }

  int Util::nextPowerOfTwo(int number) {
	if (number < 0)
	  throw std::out_of_range{
		"Util::NextPowerOfTwo => Negative numbers are not supported."};
	int ceil = number - 1;
	for (std::size_t i = 1; i < sizeof(ceil) * std::numeric_limits<unsigned char>::digits; i *= 2) {
	  ceil |= ceil >> i;
	}
	++ceil;
	return ceil;
  }
  float Util::pow(float number, float power) { return powf(number, power); }
  float Util::repeat(float t, float length) {
	return t - floorf(t / length) * length;
  }
  float Util::round(float number) { return roundf(number); }
  int Util::roundToInt(float number) { return static_cast<int>(roundf(number)); }
  int Util::sign(float number) { return (number > 0) - (number < 0); }
  float Util::sin(float radian) { return sinf(radian); }
  float Util::smoothStep(float start, float end, float number) {
	number = Util::clamp01((number - start) / (end - start));
	return square(number) * (3 - 2 * number);
  }
  float Util::sqrt(float number) { return sqrtf(number); }
  float Util::square(float number) { return number * number; }
  int Util::square(int number) { return number * number; }
  float Util::tan(float radian) { return tanf(radian); }
  bool Util::fuzzyEquals(float a, float b) { return abs(a - b) < EPSILON; }

}
