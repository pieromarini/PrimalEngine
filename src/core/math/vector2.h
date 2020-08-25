#include <string>

#include "core/math/util.h"

namespace primal::math {
  class Vector2 {
	public:
	  static const Vector2 zero;
	  static const Vector2 one;
	  static const Vector2 up;
	  static const Vector2 right;
	  static const Vector2 down;
	  static const Vector2 left;
	  static const int ELEMENT_COUNT = 2;

	  union {
		struct {
		  float x, y;
		};
		float xy[ELEMENT_COUNT];
	  };

	  Vector2() : x{ 0 }, y{ 0 } {}
	  explicit Vector2(float value) : x{ value }, y{ value } {}
	  Vector2(float inX, float inY);

	  Vector2(const Vector2& inVector) : x{ inVector.x }, y{ inVector.y } {}
	  Vector2(Vector2&& inVector) noexcept : x{ inVector.x }, y{ inVector.y } {}
	  Vector2& operator=(const Vector2& inVector) {
		x = inVector.x;
		y = inVector.y;
		return *this;
	  }

	  Vector2& operator=(Vector2&& inVector) noexcept {
		x = inVector.x;
		y = inVector.y;
		return *this;
	  }

	  ~Vector2() {}

	  float operator[](int i) const;
	  inline bool operator==(const Vector2& rhs) const {
		return x == rhs.x && y == rhs.y;
	  }

	  inline bool operator!=(const Vector2& rhs) const { return !(*this == rhs); }

	  inline Vector2 operator+(const Vector2& rhs) const {
		return Vector2(x + rhs.x, y + rhs.y);
	  }

	  inline Vector2& operator+=(const Vector2& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	  }

	  inline Vector2 operator-(const Vector2& rhs) const {
		return Vector2(x - rhs.x, y - rhs.y);
	  }

	  inline friend Vector2 operator-(float, Vector2 rhs) {
		return Vector2(-rhs.x, -rhs.y);
	  }

	  inline Vector2& operator-=(const Vector2& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	  }

	  inline Vector2 operator*(float scalar) const {
		return Vector2(x * scalar, y * scalar);
	  }

	  inline friend Vector2 operator*(float scalar, Vector2 in) {
		return in * scalar;
	  }

	  inline Vector2& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	  }

	  inline Vector2 operator/(float scalar) const {
		return Vector2(x / scalar, y / scalar);
	  }

	  inline Vector2& operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		return *this;
	  }

	  float magnitude() const;
	  float sqrMagnitude() const;
	  Vector2 normalized() const;
	  void normalize() noexcept;
	  std::string toString() const;

	  static bool equals(const Vector2& lhs, const Vector2& rhs);
	  static float dot(const Vector2& lhs, const Vector2& rhs);
	  static float cross(const Vector2& lhs, const Vector2& rhs);
	  static Vector2 lerp(const Vector2& start, const Vector2& end, float time);
	  static float distance(const Vector2& start, const Vector2& end);
	  static Vector2 project(const Vector2& inVector, const Vector2& onNormal);
	  static Vector2 reflect(const Vector2& inVector, const Vector2& inNormal);
	  static Vector2 scale(const Vector2& inVector, const Vector2& scalar);

	  static Vector2 slerp(const Vector2& start, const Vector2& end, float time);
	  static bool fuzzyEqual(const Vector2& lhs, const Vector2& rhs);
  };

}
