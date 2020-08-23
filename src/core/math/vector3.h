#ifndef VECTOR3_H
#define VECTOR3_H

#include <string>

namespace primal::math {

  class Vector3 {
	public:
	  static const Vector3 zero;
	  static const Vector3 one;
	  static const Vector3 up;
	  static const Vector3 right;
	  static const Vector3 forward;
	  static const Vector3 down;
	  static const Vector3 left;
	  static const Vector3 back;
	  static const int ELEMENT_COUNT = 3;

	  union {
		struct {
		  float x, y, z;
		};
		float xyz[ELEMENT_COUNT];
	  };

	  Vector3() : x{0}, y{0}, z{0} {}

	  explicit Vector3(float value) : x{value}, y{value}, z{value} {}

	  Vector3(float inX, float inY, float inZ) : x{inX}, y{inY}, z{inZ} {}

	  Vector3(const Vector3& inVector) : x{inVector.x}, y{inVector.y}, z{inVector.z} {}

	  Vector3(Vector3&& inVector) noexcept : x{inVector.x}, y{inVector.y}, z{inVector.z} {}

	  inline Vector3& operator=(const Vector3& inVector) {
		x = inVector.x;
		y = inVector.y;
		z = inVector.z;
		return *this;
	  }

	  inline Vector3& operator=(Vector3&& inVector) noexcept {
		x = inVector.x;
		y = inVector.y;
		z = inVector.z;
		return *this;
	  }

	  ~Vector3() {}

	  float operator[](int i) const;

	  float& operator[](int i);

	  inline bool operator==(const Vector3& rhs) const {
		return x == rhs.x && y == rhs.y && z == rhs.z;
	  }

	  inline bool operator!=(const Vector3& rhs) const { return !(*this == rhs); }

	  inline Vector3 operator+(const Vector3& rhs) const {
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	  }

	  inline Vector3& operator+=(const Vector3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	  }

	  inline Vector3 operator-(const Vector3& rhs) const {
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	  }

	  inline Vector3 operator-() const { return Vector3(-x, -y, -z); }

	  inline Vector3& operator-=(const Vector3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	  }

	  inline Vector3 operator*(float scalar) const {
		return Vector3(x * scalar, y * scalar, z * scalar);
	  }

	  inline friend Vector3 operator*(float scalar, Vector3 v) {
		return v * scalar;
	  }

	  inline Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	  }

	  inline Vector3 operator/(float scalar) const {
		return Vector3(x / scalar, y / scalar, z / scalar);
	  }

	  inline friend Vector3 operator/(float scalar, Vector3 v) {
		return Vector3(scalar / v.x, scalar / v.y, scalar / v.z);
	  }

	  inline Vector3& operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	  }

	  float magnitude() const;

	  float sqrMagnitude() const;

	  Vector3 normalized() const;

	  void normalize() noexcept;

	  std::string toString() const;

	  float max() const;
	  float min() const;

	  static bool fuzzyEqual(const Vector3& lhs, const Vector3& rhs);

	  static float dot(const Vector3& lhs, const Vector3& rhs);

	  static Vector3 cross(const Vector3& lhs, const Vector3& rhs);

	  static Vector3 lerp(const Vector3& start, const Vector3& end, float time);

	  static float distance(const Vector3& start, const Vector3& end);

	  static Vector3 project(const Vector3& inVector, const Vector3& onNormal);

	  static Vector3 reflect(const Vector3& inVector, const Vector3& inNormal);

	  static Vector3 scale(const Vector3& a, const Vector3& b);

	  static Vector3 reverseScale(const Vector3& a, const Vector3& b);

	  static Vector3 slerp(const Vector3& start, const Vector3& end, float time);

	  static Vector3 fromString(const std::string_view str);

	  static float angleDeg(const Vector3& a, const Vector3& b);

	  static float angleRad(const Vector3& a, const Vector3& b);
  };

}

#endif
