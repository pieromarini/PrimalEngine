#include <cmath>
#include <sstream>
#include <stdexcept>

#include "vector3.h"
#include "util.h"

namespace primal::math {

  const Vector3 Vector3::zero = Vector3();
  const Vector3 Vector3::one = Vector3(1.f);
  const Vector3 Vector3::up = Vector3(0.f, 1.f, 0.f);
  const Vector3 Vector3::right = Vector3(-1.f, 0.f, 0.f);
  const Vector3 Vector3::forward = Vector3(0.f, 0.f, 1.f);
  const Vector3 Vector3::down = Vector3(0.f, -1.f, 0.f);
  const Vector3 Vector3::left = Vector3(1.f, 0.f, 0.f);
  const Vector3 Vector3::back = Vector3(0.f, 0.f, -1.f);

  float Vector3::operator[](int i) const {
	if (i < 0 || i > ELEMENT_COUNT - 1)
	  throw std::out_of_range{ "Vector3::[]: Index access out of range." };
	return xyz[i];
  }

  float& Vector3::operator[](int i) {
	if (i < 0 || i > ELEMENT_COUNT - 1)
	  throw std::out_of_range{ "Vector3::[]: Index access out of range." };
	return xyz[i];
  }

  float Vector3::magnitude() const { return sqrtf(sqrMagnitude()); }
  float Vector3::sqrMagnitude() const { return x * x + y * y + z * z; }
  Vector3 Vector3::normalized() const {
	float length{ magnitude() };
	return Vector3(x / length, y / length, z / length);
  }
  void Vector3::normalize() noexcept {
	float length{ magnitude() };
	x /= length;
	y /= length;
	z /= length;
  }

  std::string Vector3::toString() const {
	std::ostringstream oss;
	oss.precision(3);
	oss << std::fixed;
	oss << "(" << x << ", " << y << ", " << z << ")";
	return oss.str();
  }

  float Vector3::max() const { return Util::max({ x, y, z }); }
  float Vector3::min() const { return Util::min({ x, y, z }); }

  bool Vector3::fuzzyEqual(const Vector3& lhs, const Vector3& rhs) {
	return Util::abs(lhs.x - rhs.x) < Util::EPSILON && Util::abs(lhs.y - rhs.y) < Util::EPSILON && Util::abs(lhs.z - rhs.z) < Util::EPSILON;
  }
  float Vector3::dot(const Vector3& lhs, const Vector3& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
  }
  Vector3 Vector3::cross(const Vector3& lhs, const Vector3& rhs) {
	return Vector3(lhs.y * rhs.z - lhs.z * rhs.y, -lhs.x * rhs.z + lhs.z * rhs.x, lhs.x * rhs.y - lhs.y * rhs.x);
  }
  Vector3 Vector3::lerp(const Vector3& start, const Vector3& end, float time) {
	return start * (1.f - time) + end * time;
  }
  float Vector3::distance(const Vector3& start, const Vector3& end) {
	return (start - end).magnitude();
  }
  Vector3 Vector3::project(const Vector3& inVector, const Vector3& onNormal) {
	return onNormal.normalized() * dot(inVector, onNormal);
  }
  Vector3 Vector3::reflect(const Vector3& inVector, const Vector3& inNormal) {
	Vector3 normal{ inNormal.normalized() };
	return inVector - normal * dot(inVector, normal) * 2.f;
  }
  Vector3 Vector3::scale(const Vector3& a, const Vector3& b) {
	return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
  }

  Vector3 Vector3::reverseScale(const Vector3& a, const Vector3& b) {
	return Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
  }

  Vector3 Vector3::slerp(const Vector3& start, const Vector3& end, float time) {
	float dotV = dot(start, end);
	dotV = dotV < -1.f ? -1.f : (dotV > 1.f ? 1.f : dotV);
	float theta = acosf(dotV) * time;
	Vector3 relativeVector = end - start * dotV;
	return start * cosf(theta) + relativeVector * sinf(theta);
  }
  Vector3 Vector3::fromString(const std::string_view str) {
	Vector3 vec;
	char c;
	std::istringstream is(str.data());
	(is >> std::skipws) >> c >> vec.x >> c >> vec.y >> c >> vec.z;

	return vec;
  }

  float Vector3::angleDeg(const Vector3& a, const Vector3& b) {
	return angleRad(a, b) * Util::RAD2DEG;
  }

  float Vector3::angleRad(const Vector3& a, const Vector3& b) {
	return Util::acos(dot(a, b) / (a.magnitude() * b.magnitude()));
  }

}
