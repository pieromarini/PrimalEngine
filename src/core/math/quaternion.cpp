#include "quaternion.h"
#include <sstream>

#include "matrix4.h"
#include "matrix3.h"
#include "vector3.h"
#include "util.h"

namespace primal::math {

  const Quaternion Quaternion::identity = Quaternion{ 0, 0, 0, 1 };

  Quaternion::Quaternion() : w{ 0.f }, x{ 0.f }, y{ 0.f }, z{ 0.f } {}

  Quaternion::Quaternion(const float inX, const float inY, const float inZ, const float inW)
	: w{ inW }, x{ inX }, y{ inY }, z{ inZ } {}

  Quaternion::Quaternion(float eulerX, float eulerY, float eulerZ) {
	eulerX *= Util::DEG2RAD;
	eulerY *= Util::DEG2RAD;
	eulerZ *= Util::DEG2RAD;
	Quaternion roll(Util::sin(eulerX * 0.5f), 0, 0, Util::cos(eulerX * 0.5f));
	Quaternion pitch(0, Util::sin(eulerY * 0.5f), 0, Util::cos(eulerY * 0.5f));
	Quaternion yaw(0, 0, Util::sin(eulerZ * 0.5f), Util::cos(eulerZ * 0.5f));

	// Order: z * y * x
	*this = (yaw * pitch * roll).normalized();
  }

  Quaternion Quaternion::fromEulerAngles(const Vector3& eulerAngles) {
	return Quaternion{ eulerAngles.x, eulerAngles.y, eulerAngles.z };
  }

  Quaternion Quaternion::fromEulerAngles(const float eulerX, const float eulerY, const float eulerZ) {
	return Quaternion{ eulerX, eulerY, eulerZ };
  }

  Quaternion Quaternion::fromAngleAxis(const Vector3& axis,
	  const float angleDeg) {
	float rad = angleDeg * Util::DEG2RAD;
	rad /= 2;
	float sin = Util::sin(rad);
	float cos = Util::cos(rad);
	return { axis.x * sin, axis.y * sin, axis.z * sin, cos };
  }

  Quaternion::Quaternion(const Vector3 vector, const float scalar) : w{ scalar }, x{ vector.x }, y{ vector.y }, z{ vector.z } {
	  normalize();
	}

  Quaternion Quaternion::fromLookRotation(const Vector3& forwardDirection, const Vector3& upDirection) {
	Quaternion ret{};
	ret.setLookRotation(forwardDirection, upDirection);
	return ret;
  }

  Quaternion::Quaternion(const Quaternion& inQuaternion) : x{ inQuaternion.x }, y{ inQuaternion.y }, z{ inQuaternion.z }, w{ inQuaternion.w } {}

  Quaternion::Quaternion(Quaternion&& inQuaternion) noexcept : x{ inQuaternion.x }, y{ inQuaternion.y }, z{ inQuaternion.z }, w{ inQuaternion.w } {}

  Quaternion& Quaternion::operator=(const Quaternion& inQuaternion) {
	x = inQuaternion.x;
	y = inQuaternion.y;
	z = inQuaternion.z;
	w = inQuaternion.w;

	return *this;
  }

  Quaternion& Quaternion::operator=(Quaternion&& inQuaternion) noexcept {
	x = inQuaternion.x;
	y = inQuaternion.y;
	z = inQuaternion.z;
	w = inQuaternion.w;

	return *this;
  }

  bool Quaternion::operator==(const Quaternion& rhs) const {
	return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
  }

  bool Quaternion::operator!=(const Quaternion& rhs) const {
	return !(*this == rhs);
  }

  Quaternion Quaternion::operator+(const Quaternion& rhs) const {
	return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
  }

  Quaternion& Quaternion::operator+=(const Quaternion& rhs) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
  }

  Quaternion Quaternion::operator-(const Quaternion& rhs) const {
	return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
  }

  Quaternion& Quaternion::operator-=(const Quaternion& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
  }

  Quaternion Quaternion::operator*(const Quaternion& rhs) const {
	return { y * rhs.z - z * rhs.y + rhs.x * w + x * rhs.w,
		z * rhs.x - x * rhs.z + rhs.y * w + y * rhs.w,
		x * rhs.y - y * rhs.x + rhs.z * w + z * rhs.w,
		w * rhs.w - (x * rhs.x + y * rhs.y + z * rhs.z) };
  }

  Quaternion& Quaternion::operator*=(const Quaternion& rhs) {
	return *this = *this * rhs;
  }

  Quaternion Quaternion::operator*(float scalar) const {
	return { x * scalar, y * scalar, z * scalar, w * scalar };
  }

  Vector3 Quaternion::operator*(const Vector3& rhs) const {
	auto quat = *this * Quaternion{ rhs.x, rhs.y, rhs.z, 0 } * inverse(*this);
	return Vector3{ quat.x, quat.y, quat.z };
  }

  Quaternion::operator Matrix4() {
	Matrix4 ret = Matrix4::identity;
	ret.setTopLeftMatrix3(getMatrix3());
	return ret;
  }

  Vector3 Quaternion::getEulerAngles() const {
	Quaternion q = (*this).normalized();

	// roll (x-axis rotation)
	float sinRoll = 2.f * (q.w * q.x + q.y * q.z);
	float cosRoll = 1.f - 2.f * (q.x * q.x + q.y * q.y);
	float roll = Util::atan2(sinRoll, cosRoll);

	// pitch (y-axis rotation)
	float sinPitch = 2.f * (q.w * q.y - q.z * q.x);
	float pitch;
	if (Util::abs(sinPitch) >= 1)
	  pitch =
		Util::PI / 2 * Util::sign(sinPitch);// use 90 degrees if out of range
	else
	  pitch = Util::asin(sinPitch);

	// yaw (z-axis rotation)
	float sinYaw = 2.f * (q.w * q.z + q.x * q.y);
	float cosYaw = 1.f - 2.f * (q.y * q.y + q.z * q.z);
	float yaw = Util::atan2(sinYaw, cosYaw);

	Vector3 angleInRad = Vector3{ roll, pitch, yaw };
	return angleInRad * Util::RAD2DEG;
  }

  void Quaternion::setFromToRotation(const Vector3& fromDirection,
	  const Vector3& toDirection) {
	float dot = Vector3::dot(fromDirection, toDirection);
	float k =
	  Util::sqrt(fromDirection.sqrMagnitude() * toDirection.sqrMagnitude());
	if (Util::abs(dot / k + 1) < Util::EPSILON) {
	  Vector3 ortho;
	  if (fromDirection.z < fromDirection.x) {
		ortho = Vector3{ fromDirection.y, -fromDirection.x, 0.f };
	  } else {
		ortho = Vector3{ 0.f, -fromDirection.z, fromDirection.y };
	  }
	  *this = Quaternion(ortho.normalized(), 0.f);
	}
	Vector3 cross = Vector3::cross(fromDirection, toDirection);
	*this = Quaternion(cross, dot + k).normalized();
  }

  void Quaternion::setLookRotation(const Vector3& forwardDirection,
	  const Vector3& upDirection) {
	// Normalize inputs
	Vector3 forward = forwardDirection.normalized();
	Vector3 upwards = upDirection.Normalized();
	// Don't allow zero vectors
	if (Vector3::fuzzyEqual(forward, Vector3::zero) || Vector3::fuzzyEqual(upwards, Vector3::zero)) {
	  x = 0.f;
	  y = 0.f;
	  z = 0.f;
	  w = 0.f;
	}
	// Handle alignment with up direction
	if (1 - Util::abs(Vector3::dot(forward, upwards)) < Util::EPSILON)
	  setFromToRotation(Vector3::forward, forward);
	// Get orthogonal vectors
	Vector3 right = Vector3::cross(upwards, forward).Normalized();
	upwards = Vector3::cross(forward, right);
	// Calculate rotation
	float radicand = right.x + upwards.y + forward.z;
	if (radicand > 0.f) {
	  w = Util::sqrt(1.f + radicand) * 0.5f;
	  float recip = 1.f / (4.f * w);
	  x = (upwards.z - forward.y) * recip;
	  y = (forward.x - right.z) * recip;
	  z = (right.y - upwards.x) * recip;
	} else if (right.x >= upwards.y && right.x >= forward.z) {
	  x = Util::sqrt(1.f + right.x - upwards.y - forward.z) * 0.5f;
	  float recip = 1.f / (4.f * x);
	  w = (upwards.z - forward.y) * recip;
	  z = (forward.x + right.z) * recip;
	  y = (right.y + upwards.x) * recip;
	} else if (upwards.y > forward.z) {
	  y = Util::sqrt(1.f - right.x + upwards.y - forward.z) * 0.5f;
	  float recip = 1.f / (4.f * y);
	  z = (upwards.z + forward.y) * recip;
	  w = (forward.x - right.z) * recip;
	  x = (right.y + upwards.x) * recip;
	} else {
	  z = Util::sqrt(1.f - right.x - upwards.y + forward.z) * 0.5f;
	  float recip = 1.f / (4.f * z);
	  y = (upwards.z + forward.y) * recip;
	  x = (forward.x + right.z) * recip;
	  w = (right.y - upwards.x) * recip;
	}
	normalize();
  }

  Quaternion Quaternion::normalized() const {
	Quaternion ret{ x, y, z, w };
	float length = ret.x * ret.x + ret.y * ret.y + ret.z * ret.z + ret.w * ret.w;
	ret.x /= length;
	ret.y /= length;
	ret.z /= length;
	ret.w /= length;
	return ret;
  }

  void Quaternion::normalize() {
	float length = x * x + y * y + z * z + w * w;
	x /= length;
	y /= length;
	z /= length;
	w /= length;
  }

  std::string Quaternion::toString() const {
	std::ostringstream oss;
	oss << "(" << x << ", " << y << ", " << z << ", " << w << ")";
	return oss.str();
  }

  // reference: Game Engine Architecture 2nd edition page 205
  Matrix3 Quaternion::getMatrix3() const {
	return { 1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w, 2 * x * z + 2 * y * w, 2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w, 1 - 2 * x * x - 2 * y * y };
  }

  Quaternion Quaternion::getInverse() const { return inverse(*this); }

  float Quaternion::angleRad(const Quaternion& aQuaternion,
	  const Quaternion& bQuaternion) {
	float dotV = dot(aQuaternion, bQuaternion);
	return Util::acos(Util::min({ Util::abs(dotV), 1.f })) * 2.f;
  }

  float Quaternion::angleDeg(const Quaternion& aQuaternion, const Quaternion& bQuaternion) {
	return angleRad(aQuaternion, bQuaternion) * Util::RAD2DEG;
  }

  float Quaternion::dot(const Quaternion& aQuaternion, const Quaternion& bQuaternion) {
	return aQuaternion.x * bQuaternion.x + aQuaternion.y * bQuaternion.y + aQuaternion.z * bQuaternion.z + aQuaternion.w * bQuaternion.w;
  }

  Quaternion Quaternion::inverse(const Quaternion& quaternion) {
	float length = quaternion.x * quaternion.x + quaternion.y * quaternion.y + quaternion.z * quaternion.z + quaternion.w * quaternion.w;
	return Quaternion(quaternion.x / -length, quaternion.y / -length, quaternion.z / -length, quaternion.w / length);
  }

  Quaternion Quaternion::lerp(const Quaternion& aQuaternion,
	  const Quaternion& bQuaternion,
	  const float t) {
	return (aQuaternion * (1.f - t) + bQuaternion * t).normalized();
  }

  Quaternion Quaternion::slerp(const Quaternion& aQuaternion,
	  const Quaternion& bQuaternion,
	  float t) {
	if (aQuaternion == bQuaternion) {
	  return aQuaternion.normalized();
	}
	float theta{ Util::acos(dot(aQuaternion, bQuaternion)) };
	theta = abs(theta);

	float wp = Util::sin((1 - t) * theta) / Util::sin(theta);
	float wq = Util::sin(t * theta) / Util::sin(theta);
	return (aQuaternion * wp + bQuaternion * wq).normalized();
  }

  bool Quaternion::fuzzyEqual(const Quaternion& lhs, const Quaternion& rhs) {
	return abs(lhs.x - rhs.x) < Util::EPSILON && abs(lhs.y - rhs.y) < Util::EPSILON && abs(lhs.z - rhs.z) < Util::EPSILON && abs(lhs.w - rhs.w) < Util::EPSILON;
  }

  Quaternion operator""_i(long double inX) {
	return { static_cast<float>(inX), 0.f, 0.f, 0.f };
  }

  Quaternion operator""_j(long double inY) {
	return { 0.f, static_cast<float>(inY), 0.f, 0.f };
  }

  Quaternion operator""_k(long double inZ) {
	return { 0.f, 0.f, static_cast<float>(inZ), 0.f };
  }

  Quaternion operator""_w(long double inW) {
	return { 0.f, 0.f, 0.f, static_cast<float>(inW) };
  }

}
