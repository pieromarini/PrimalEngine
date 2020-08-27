#ifndef MATH_QUATERNION_H
#define MATH_QUATERNION_H

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

	vec4 ToAxisAngle();
	mat3 ToMatrix();

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

  inline vec4 quaternion::ToAxisAngle() {
	vec4 result;

	const float angle = 2.0f * acos(w);
	const float length = sqrt(1.0f - angle * angle);

	result.xyz = vec3(x, y, z) / length;
	result.w = angle;

	return result;
  }

  inline mat3 quaternion::ToMatrix() {
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

}

#endif
