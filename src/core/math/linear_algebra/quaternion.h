#ifndef MATH_QUATERNION_H
#define MATH_QUATERNION_H

#include "core/math/trigonometry/conversions.h"
#include "core/math/common.h"
#include "vector.h"
#include "operation.h"

#include <cmath>

namespace primal::math {
  /*
	A Quaternion is a 4-dimensional vector consisting of
	a scalar part w and a 3-dimensional semi-rotation 
	axis v: { w, v } or { w, v_x, v_y, v_z }.
	Quaternions are an efficient and precise tool for
	representing rotations in 3D space as an angle-axis
	representation with w being the angle/2 and the 
	vector v being the axis of rotation, but normalized
	and scaled by sin(angle/2).
	Note that there is no need for templates here, a 
	quaternion is always a 4-dimensional entity and as 
	we always use quaternions within the range [-1,1] 
	float precision is sufficient enough.
  */
  struct quaternion {
	float w;
	union {
	  struct
	  {
		float x, y, z;
	  };
	  vec3 r;
	};

	quaternion() : w(0.0f), x(0.0f), y(0.0f), z(0.0f) { }

	quaternion(const float w, const float x, const float y, const float z) : w(w), x(x), y(y), z(z) { }

	quaternion(const vec3& axis, const float radians) {
	  const float halfAngle = 0.5f * radians;
	  w = cos(halfAngle);
	  x = axis.x * sin(halfAngle);
	  y = axis.y * sin(halfAngle);
	  z = axis.z * sin(halfAngle);
	}

	explicit quaternion(const vec3& axis) {
	  w = 0.0f;
	  x = axis.x;
	  y = axis.y;
	  z = axis.z;
	}

	void setLookRotation(const vec3& forwardDirection, const vec3& upDirection);
	void setFromToRotation(const vec3& fromDirection, const vec3& toDirection);

	[[nodiscard]] mat3 getMatrix3() const;

	static quaternion fromEulerAngles(const vec3 eulerAngles);
	static quaternion fromLookRotation(const vec3& forwardDirection, const vec3& upDirection);

	[[nodiscard]] quaternion getInverse() const { return inverse(*this); }

	[[nodiscard]] vec4 toAxisAngle() const;
	[[nodiscard]] mat3 toMatrix() const;
	[[nodiscard]] vec3 getEulerAngles() const;

	[[nodiscard]] quaternion inverse(const quaternion& quaternion) const;

	quaternion operator-();
  };

  inline quaternion quaternion::operator-() {
	return quaternion(-w, -x, -y, -z);
  }

  inline quaternion operator+(const quaternion& lhs, const quaternion& rhs) {
	return quaternion(lhs.w + rhs.w, lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
  }

  inline quaternion operator*(const quaternion& lhs, const float scalar) {
	return quaternion(scalar * lhs.w, scalar * lhs.x, scalar * lhs.y, scalar * lhs.z);
  }
  inline quaternion operator*(const float scalar, const quaternion& rhs) {
	return quaternion(scalar * rhs.w, scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
  }

  inline float length(const quaternion& quat) {
	return sqrt(quat.w * quat.w + quat.x * quat.x + quat.y * quat.y + quat.z * quat.z);
  }

  inline quaternion& normalize(quaternion& quat) {
	const float l = length(quat);
	quat = quat * (1.0f / l);
	return quat;
  }

  inline float dot(const quaternion& lhs, const quaternion& rhs) {
	return lhs.w * rhs.w + lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
  }

  inline quaternion operator*(const quaternion& lhs, const quaternion& rhs) {
	vec3 v1(rhs.x, rhs.y, rhs.z);
	vec3 v2(lhs.x, lhs.y, lhs.z);

	const float w = rhs.w * lhs.w - dot(rhs, lhs);
	vec3 v = rhs.w * v2 + lhs.w * v1 + cross(v2, v1);

	return quaternion(w, v.x, v.y, v.z);
  }

  inline vec3 operator*(const quaternion& quat, const vec3& vec) {
	const float w2 = quat.w * quat.w;

	return (2.0f * w2 - 1.0f) * vec + 2.0f * dot(quat.r, vec) * quat.r + w2 * cross(quat.r, vec);
  }

  inline quaternion inverse(const quaternion& quat) {
	// TODO: assert that we're dealing with a unity vector
	return quaternion(quat.w, -quat.x, -quat.y, -quat.z);
  }

  inline vec4 quaternion::toAxisAngle() const {
	vec4 result;

	const float angle = 2.0f * acos(w);
	const float length = sqrt(1.0f - angle * angle);

	result.xyz = vec3(x, y, z) / length;
	result.w = angle;

	return result;
  }

  inline vec3 quaternion::getEulerAngles() const {
	auto copy = *this;
	auto q = normalize(copy);

	// roll (x-axis rotation)
	float sinRoll = 2.f * (q.w * q.x + q.y * q.z);
	float cosRoll = 1.f - 2.f * (q.x * q.x + q.y * q.y);
	float roll = std::atan2(sinRoll, cosRoll);

	// pitch (y-axis rotation)
	float sinPitch = 2.f * (q.w * q.y - q.z * q.x);
	float pitch{};
	if (std::abs(sinPitch) >= 1)
	  pitch = PI / 2 * ((sinPitch > 0) - (sinPitch < 0));
	else
	  pitch = std::asin(sinPitch);

	// yaw (z-axis rotation)
	float sinYaw = 2.f * (q.w * q.z + q.x * q.y);
	float cosYaw = 1.f - 2.f * (q.y * q.y + q.z * q.z);
	float yaw = std::atan2(sinYaw, cosYaw);

	vec3 angleInDeg{ rad2Deg(roll), rad2Deg(pitch), rad2Deg(yaw) };
	return angleInDeg;
  }

  inline mat3 quaternion::toMatrix() const {
	mat3 mat;

	mat[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	mat[0][1] = 2.0f * x * y + 2.0f * w * z;
	mat[0][2] = 2.0f * x * z - 2.0f * w * y;

	mat[1][0] = 2.0f * x * y - 2.0f * w * z;
	mat[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	mat[1][2] = 2.0f * y * z + 2.0f * w * x;

	mat[2][0] = 2.0f * x * z + 2.0f * w * y;
	mat[2][1] = 2.0f * y * z - 2.0f * w * x;
	mat[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;

	return mat;
  }

  inline quaternion quaternion::inverse(const quaternion& quaternion) const {
	float length = quaternion.x * quaternion.x + quaternion.y * quaternion.y +
	  quaternion.z * quaternion.z + quaternion.w * quaternion.w;
	return { quaternion.x / -length,
			 quaternion.y / -length,
		     quaternion.z / -length,
			 quaternion.w / length };
  }

  inline quaternion quaternion::fromEulerAngles(const vec3 eulerAngles) {
	auto eulerX = deg2Rad(eulerAngles.x);
	auto eulerY = deg2Rad(eulerAngles.y);
	auto eulerZ = deg2Rad(eulerAngles.z);

	quaternion roll(std::sin(eulerX * 0.5f), 0, 0, std::cos(eulerX * 0.5f));
	quaternion pitch(0, std::sin(eulerY * 0.5f), 0, std::cos(eulerY * 0.5f));
	quaternion yaw(0, 0, std::sin(eulerZ * 0.5f), std::cos(eulerZ * 0.5f));

	auto tmp = roll * pitch * yaw;
	return normalize(tmp);
  }

  inline quaternion quaternion::fromLookRotation(const vec3& forwardDirection, const vec3& upDirection) {
	quaternion ret{};
	ret.setLookRotation(forwardDirection, upDirection);
	return ret;
  }

  inline void quaternion::setFromToRotation(const vec3& fromDirection, const vec3& toDirection) {
	float dotValue = dot(fromDirection, toDirection);
	float k = std::sqrt(lengthSquared(fromDirection) * lengthSquared(toDirection));
	if (std::abs(dotValue / k + 1) < EPSILON) {
	  vec3 ortho;
	  if (fromDirection.z < fromDirection.x) {
		ortho = vec3{fromDirection.y, -fromDirection.x, 0.f};
	  } else {
		ortho = vec3{0.f, -fromDirection.z, fromDirection.y};
	  }
	  *this = quaternion(normalize(ortho), 0.f);
	}
	auto crossValue = cross(fromDirection, toDirection);
	auto q = quaternion(crossValue, dotValue + k);
	*this = normalize(q);
  }

  inline void quaternion::setLookRotation(const vec3& forwardDirection, const vec3& upDirection) {
	// Normalize inputs
	auto forward = normalize(forwardDirection);
	auto upwards = normalize(upDirection);

	if (1 - std::abs(dot(forward, upwards)) < EPSILON)
	  setFromToRotation(vec3::FORWARD, forward);
	// Get orthogonal vectors
	auto right = normalize(cross(upwards, forward));
	upwards = cross(forward, right);
	// Calculate rotation
	float radicand = right.x + upwards.y + forward.z;
	if (radicand > 0.f) {
	  w = std::sqrt(1.f + radicand) * 0.5f;
	  float recip = 1.f / (4.f * w);
	  x = (upwards.z - forward.y) * recip;
	  y = (forward.x - right.z) * recip;
	  z = (right.y - upwards.x) * recip;
	} else if (right.x >= upwards.y && right.x >= forward.z) {
	  x = std::sqrt(1.f + right.x - upwards.y - forward.z) * 0.5f;
	  float recip = 1.f / (4.f * x);
	  w = (upwards.z - forward.y) * recip;
	  z = (forward.x + right.z) * recip;
	  y = (right.y + upwards.x) * recip;
	} else if (upwards.y > forward.z) {
	  y = std::sqrt(1.f - right.x + upwards.y - forward.z) * 0.5f;
	  float recip = 1.f / (4.f * y);
	  z = (upwards.z + forward.y) * recip;
	  w = (forward.x - right.z) * recip;
	  x = (right.y + upwards.x) * recip;
	} else {
	  z = std::sqrt(1.f - right.x - upwards.y + forward.z) * 0.5f;
	  float recip = 1.f / (4.f * z);
	  y = (upwards.z + forward.y) * recip;
	  x = (forward.x + right.z) * recip;
	  w = (right.y - upwards.x) * recip;
	}
	*this = normalize(*this);
  }

  inline mat3 quaternion::getMatrix3() const {
	return {1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w,
			2 * x * z + 2 * y * w,     2 * x * y + 2 * z * w,
			1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w,
			2 * x * z - 2 * y * w,     2 * y * z + 2 * x * w,
			1 - 2 * x * x - 2 * y * y};
  }

}

#endif
