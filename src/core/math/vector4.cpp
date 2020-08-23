#include <cmath>
#include <stdexcept>

#include "vector4.h"
#include "vector3.h"
#include "util.h"

namespace primal::math {

  const Vector4 Vector4::zero = Vector4();
  const Vector4 Vector4::one = Vector4(1.f);
  const Vector4 Vector4::right = Vector4{ -1.0f, 0, 0, 0 };
  const Vector4 Vector4::left = Vector4{ 1.0f, 0, 0, 0 };
  const Vector4 Vector4::up = Vector4{ 0, 1.0f, 0, 0 };
  const Vector4 Vector4::down = Vector4{ 0, -1.0f, 0, 0 };
  const Vector4 Vector4::forward = Vector4{ 0, 0, 1.0f, 0 };
  const Vector4 Vector4::back = Vector4{ 0, 0, -1.0f, 0 };

  Vector4::Vector4(const Vector3& inVector, float inW) : x{ inVector.x }, y{ inVector.y }, z{ inVector.z }, w{ inW } {}

  Vector4::operator class Vector3() const { return Vector3(x, y, z); }

  float Vector4::operator[](int i) const {
	if (i < 0 || i > ELEMENT_COUNT - 1)
	  throw std::out_of_range{ "Vector4::[]: Index access out of range." };
	return xyzw[i];
  }

  float& Vector4::operator[](int i) {
	if (i < 0 || i > ELEMENT_COUNT - 1)
	  throw std::out_of_range{ "Vector4::[]: Index access out of range." };
	return xyzw[i];
  }

  void Vector4::set(const Vector3& inXYZ, const float inW) {
	x = inXYZ.x;
	y = inXYZ.y;
	z = inXYZ.z;
	w = inW;
  }

  Vector3 Vector4::getVector3() const { return Vector3{ x, y, z }; }

  float Vector4::magnitude() const { return sqrtf(sqrMagnitude()); }

  float Vector4::sqrMagnitude() const { return x * x + y * y + z * z + w * w; }

  Vector4 Vector4::normalized() const {
	float length{ magnitude() };
	return Vector4(x / length, y / length, z / length, w / length);
  }

  void Vector4::normalize() noexcept {
	float length{ magnitude() };
	x /= length;
	y /= length;
	z /= length;
	w /= length;
  }

  bool Vector4::fuzzyEqual(const Vector4& lhs, const Vector4& rhs) {
	return abs(lhs.x - rhs.x) < Util::EPSILON && abs(lhs.y - rhs.y) < Util::EPSILON && abs(lhs.z - rhs.z) < Util::EPSILON && abs(lhs.w - rhs.w) < Util::EPSILON;
  }
  float Vector4::dot(const Vector4& lhs, const Vector4& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
  }
  Vector4 Vector4::lerp(const Vector4& start, const Vector4& end, float time) {
	return start * (1.f - time) + end * time;
  }
  float Vector4::distance(const Vector4& start, const Vector4& end) {
	return (start - end).magnitude();
  }
  Vector4 Vector4::project(const Vector4& inVector, const Vector4& onNormal) {
	return onNormal.normalized() * dot(inVector, onNormal);
  }
  Vector4 Vector4::scale(const Vector4& aVector, const Vector4& bVector) {
	return Vector4(aVector.x * bVector.x, aVector.y * bVector.y, aVector.z * bVector.z, aVector.w * bVector.w);
  }
  Vector4 Vector4::slerp(const Vector4& start, const Vector4& end, float time) {
	float dotV{ dot(start, end) };
	dotV = dotV < -1.f ? -1.f : (dotV > 1.f ? 1.f : dotV);
	float theta = acosf(dotV) * time;
	Vector4 relativeVector = end - start * dotV;
	return start * cosf(theta) + relativeVector * sinf(theta);
  }

}
