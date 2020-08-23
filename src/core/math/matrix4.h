#ifndef MATRIX4_H
#define MATRIX4_H

namespace primal::math {

  class Matrix4 {
	public:
	  static const Matrix4 zero;
	  static const Matrix4 identity;
	  static const Matrix4 xRot45;
	  static const Matrix4 xRot90;
	  static const Matrix4 yRot45;
	  static const Matrix4 yRot90;
	  static const Matrix4 zRot45;
	  static const Matrix4 zRot90;

	  static const int ELEMENT_COUNT = 16;
	  static const int ROW_COUNT = 4;

	  union {
		float data[ELEMENT_COUNT];
		float row_col[ROW_COUNT][ROW_COUNT];
	  };

	  Matrix4();

	  explicit Matrix4(float value);

	  explicit Matrix4(const float inMat[]);

	  Matrix4(float m11, float m12, float m13, float m14, float m21, float m22,
		  float m23, float m24, float m31, float m32, float m33, float m34,
		  float m41, float m42, float m43, float m44);

	  explicit Matrix4(const class Quaternion& quat);

	  Matrix4(const Matrix4& inMatrix);
	  Matrix4(Matrix4&& inMatrix) noexcept;
	  Matrix4& operator=(const Matrix4& inMatrix);
	  Matrix4& operator=(Matrix4&& inMatrix) noexcept;

	  Matrix4(const class Vector4& aVector, const class Vector4& bVector);

	  ~Matrix4() = default;

	  float* operator[](int i) const;
	  bool operator==(const Matrix4& rhs) const;
	  bool operator!=(const Matrix4& rhs) const;
	  Matrix4 operator+(const Matrix4& rhs) const;
	  Matrix4& operator+=(const Matrix4& rhs);
	  Matrix4 operator-(const Matrix4& rhs) const;
	  Matrix4 operator-=(const Matrix4& rhs);
	  Matrix4 operator*(const Matrix4& rhs) const;
	  Matrix4 operator*=(const Matrix4& rhs);
	  Matrix4 operator*(float scalar) const;
	  Matrix4 operator*=(float scalar);
	  class Vector4 operator*(const class Vector4& rhs) const;

	  float determinant() const;

	  float get(int x, int y) const;

	  void set(int x, int y, float number);

	  Matrix4 inverse() const;

	  Matrix4 transpose() const;

	  bool isIdentity() const;

	  bool isZero() const;

	  class Vector4 getRow(int row) const;

	  void setRow(int row, const class Vector4& rowData);
	  void setRow(int row, const class Vector3& rowData, float rowVal);

	  class Vector4 getCol(int col) const;

	  class Matrix3 getTopLeftMatrix3() const;

	  void setCol(int col, const class Vector4& colData);
	  void setCol(int col, const class Vector3& colData, float colVal);
	  void setTopLeftMatrix3(const class Matrix3& matrix3);
	  void setDiagonal(const class Vector4& diagonal);
	  void setDiagonal(float r0c0, float r1c1, float r2c2, float r3c3);

	  static Matrix4 translate(const class Vector3& translation);

	  static Matrix4 scale(const class Vector3& scale);
	  static Matrix4 rotateX(float radians);
	  static Matrix4 rotateY(float radians);
	  static Matrix4 rotateZ(float radians);
	  static Matrix4 rotate(const class Vector3& rotation);
	  static Matrix4 transform(const class Vector3& translation, const class Vector3& rotation, const class Vector3& scale);

	  static bool FuzzyEqual(const Matrix4& lhs, const Matrix4& rhs);
  };

}

#endif
