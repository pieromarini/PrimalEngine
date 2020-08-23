#ifndef QUATERNION_H
#define QUATERNION_H

#include <string>

#include "matrix4.h"

namespace primal::math {

  class Quaternion {
	public:
	  static const Quaternion identity;

	  float w, x, y, z;

	  Quaternion();

	  Quaternion(const float inX, const float inY, const float inZ, const float inW);

	  Quaternion(float eulerX, float eulerY, float eulerZ);

	  static Quaternion fromEulerAngles(const class Vector3& eulerAngles);

	  static Quaternion fromEulerAngles(float eulerX, float eulerY, float eulerZ);

	  Quaternion(class Vector3 vector, float scalar);

	  static Quaternion fromAngleAxis(const class Vector3& axis, float angleDeg);

	  static Quaternion fromLookRotation(const class Vector3& forwardDirection, const class Vector3& upDirection);

	  Quaternion(const Quaternion& inQuaternion);
	  Quaternion(Quaternion&& inQuaternion) noexcept;
	  Quaternion& operator=(const Quaternion& inQuaternion);
	  Quaternion& operator=(Quaternion&& inQuaternion) noexcept;

	  ~Quaternion() = default;

	  bool operator==(const Quaternion& rhs) const;
	  bool operator!=(const Quaternion& rhs) const;
	  Quaternion operator+(const Quaternion& rhs) const;
	  Quaternion& operator+=(const Quaternion& rhs);
	  Quaternion operator-(const Quaternion& rhs) const;
	  Quaternion& operator-=(const Quaternion& rhs);
	  Quaternion operator*(const Quaternion& rhs) const;
	  Quaternion& operator*=(const Quaternion& rhs);
	  Quaternion operator*(float scalar) const;
	  Vector3 operator*(const Vector3& rhs) const;

	  explicit operator Matrix4();

	  class Vector3 getEulerAngles() const;

	  void setFromToRotation(const class Vector3& fromDirection, const class Vector3& toDirection);

	  void setLookRotation(const class Vector3& forwardDirection, const class Vector3& upDirection);

	  Quaternion normalized() const;

	  void normalize();

	  std::string toString() const;

	  class Matrix3 getMatrix3() const;

	  Quaternion getInverse() const;

	  static float angleRad(const Quaternion& aQuaternion, const Quaternion& bQuaternion);

	  static float angleDeg(const Quaternion& aQuaternion, const Quaternion& bQuaternion);

	  static float dot(const Quaternion& aQuaternion, const Quaternion& bQuaternion);

	  static Quaternion inverse(const Quaternion& quaternion);

	  static Quaternion lerp(const Quaternion& aQuaternion, const Quaternion& bQuaternion, float t);

	  static Quaternion slerp(const Quaternion& aQuaternion, const Quaternion& bQuaternion, float t);

	  static bool fuzzyEqual(const Quaternion& lhs, const Quaternion& rhs);
  };

  Quaternion operator"" _i(long double inX);
  Quaternion operator"" _j(long double inY);
  Quaternion operator"" _k(long double inZ);
  Quaternion operator"" _w(long double inW);

}

#endif
