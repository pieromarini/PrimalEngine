#ifndef VECTOR4_H
#define VECTOR4_H

namespace primal::math {

  class Vector4 {
	public:
	  static const Vector4 zero;
	  static const Vector4 one;
	  static const Vector4 right;
	  static const Vector4 left;
	  static const Vector4 up;
	  static const Vector4 down;
	  static const Vector4 forward;
	  static const Vector4 back;
	  static const int ELEMENT_COUNT = 4;

	  union {
		struct {
		  float x, y, z, w;
		};
		float xyzw[ELEMENT_COUNT];
	  };

	  Vector4() : x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 } {}

	  explicit Vector4(float value) : x{ value }, y{ value }, z{ value }, w{ value } {}

	  Vector4(float inX, float inY, float inZ, float inW) : x{ inX }, y{ inY }, z{ inZ }, w{ inW } {}

	  Vector4(const class Vector3& inVector, float inW);

	  Vector4(const Vector4& inVector) : x{ inVector.x }, y{ inVector.y }, z{ inVector.z }, w{ inVector.w } {}

	  Vector4(Vector4&& inVector) noexcept : x{ inVector.x }, y{ inVector.y }, z{ inVector.z }, w{ inVector.w } {}

	  inline Vector4& operator=(const Vector4& inVector) {
		x = inVector.x;
		y = inVector.y;
		z = inVector.z;
		w = inVector.w;
		return *this;
	  }

	  inline Vector4& operator=(Vector4&& inVector) noexcept {
		x = inVector.x;
		y = inVector.y;
		z = inVector.z;
		w = inVector.w;
		return *this;
	  }

	  ~Vector4() {}

	  float operator[](int i) const;

	  float& operator[](int i);

	  inline bool operator==(const Vector4& rhs) const {
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	  }

	  inline bool operator!=(const Vector4& rhs) const { return !(*this == rhs); }

	  inline Vector4 operator+(const Vector4& rhs) const {
		return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	  }

	  inline Vector4& operator+=(const Vector4& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	  }

	  inline Vector4 operator-(const Vector4& rhs) const {
		return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	  }

	  inline Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

	  inline Vector4& operator-=(const Vector4& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;
		return *this;
	  }

	  inline Vector4 operator*(float scalar) const {
		return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
	  }

	  inline friend Vector4 operator*(float scalar, Vector4 v) {
		return v * scalar;
	  }

	  inline Vector4& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	  }

	  inline Vector4 operator/(float scalar) const {
		return { x / scalar, y / scalar, z / scalar, w / scalar };
	  }

	  inline Vector4& operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;
		return *this;
	  }

	  // Conversions
	  explicit operator class Vector3() const;

	  // Functions

	  void set(const class Vector3& inXYZ, float inW);

	  Vector3 getVector3() const;

	  float magnitude() const;

	  float sqrMagnitude() const;

	  Vector4 normalized() const;

	  void normalize() noexcept;

	  static bool fuzzyEqual(const Vector4& lhs, const Vector4& rhs);

	  static float dot(const Vector4& lhs, const Vector4& rhs);

	  static Vector4 lerp(const Vector4& start, const Vector4& end, float time);

	  static float distance(const Vector4& start, const Vector4& end);

	  static Vector4 project(const Vector4& inVector, const Vector4& onNormal);

	  static Vector4 scale(const Vector4& aVector, const Vector4& bVector);

	  static Vector4 slerp(const Vector4& start, const Vector4& end, float time);
  };

}

#endif
