#include "vector2.h"
#include "tools/log.h"
#include <sstream>

namespace primal::math {

  const Vector2 Vector2::zero = Vector2();
  const Vector2 Vector2::one = Vector2(1.f);
  const Vector2 Vector2::up = Vector2(0.f, 1.f);
  const Vector2 Vector2::right = Vector2(1.f, 0.f);
  const Vector2 Vector2::down = Vector2(0.f, -1.f);
  const Vector2 Vector2::left = Vector2(-1.f, 0.f);

  Vector2::Vector2(float inX, float inY) : x{ inX }, y{ inY } {}

  float Vector2::operator[](int i) const {
	if (i < 0 || i > ELEMENT_COUNT - 1)
	  PRIMAL_CORE_ERROR("Vector2::[]: Index Access out of range");
	return xy[i];
  }

  float Vector2::magnitude() const { return sqrtf(sqrMagnitude()); }
  float Vector2::sqrMagnitude() const { return x * x + y * y; }
  Vector2 Vector2::normalized() const {
	float length{ magnitude() };
	if (length == 0) return zero;
	return Vector2(x / length, y / length);
  }
  void Vector2::normalize() noexcept {
	float length{ magnitude() };
	if (length == 0) return;
	x /= length;
	y /= length;
  }

  std::string Vector2::toString() const {
	std::ostringstream oss;
	oss << "(" << x << ", " << y << ")";
	return oss.str();
  }

  bool Vector2::equals(const Vector2& lhs, const Vector2& rhs) {
	return abs(lhs.x - rhs.x) < Util::EPSILON && abs(lhs.y - rhs.y) < Util::EPSILON;
  }
  float Vector2::dot(const Vector2& lhs, const Vector2& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y;
  }
  float Vector2::cross(const Vector2& lhs, const Vector2& rhs) {
	return lhs.x * rhs.y - lhs.y * rhs.x;
  }
  Vector2 Vector2::lerp(const Vector2& start, const Vector2& end, float time) {
	return start * (1.f - time) + end * time;
  }
  float Vector2::distance(const Vector2& start, const Vector2& end) {
	return (start - end).magnitude();
  }
  Vector2 Vector2::project(const Vector2& inVector, const Vector2& onNormal) {
	return onNormal.normalized() * dot(inVector, onNormal);
  }
  Vector2 Vector2::reflect(const Vector2& inVector, const Vector2& inNormal) {
	Vector2 normal{ inNormal.normalized() };
	return inVector - normal * dot(inVector, normal) * 2.f;
  }
  Vector2 Vector2::scale(const Vector2& inVector, const Vector2& scalar) {
	return Vector2(inVector.x * scalar.x, inVector.y * scalar.y);
  }
  Vector2 Vector2::slerp(const Vector2& start, const Vector2& end, float time) {
	float dotValue = dot(start, end);
	dotValue = dotValue < -1.f ? -1.f : (dotValue > 1.f ? 1.f : dotValue);
	float theta = acosf(dotValue) * time;
	auto relativeVector = end - start * dotValue;
	return start * cosf(theta) + relativeVector * sinf(theta);
  }

  bool Vector2::fuzzyEqual(const Vector2& lhs, const Vector2& rhs) {
	return abs(lhs.x - rhs.x) < Util::EPSILON && abs(lhs.y - rhs.y) < Util::EPSILON;
  }

}// namespace primal::math
