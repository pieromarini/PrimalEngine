#ifndef MATRIX3_H
#define MATRIX3_H

namespace primal::math {

  class Matrix3 {
	public:
	  static const Matrix3 zero;
	  static const Matrix3 identity;
	  static const int ELEMENT_COUNT = 9;
	  static const int ROW_COUNT = 3;

	  union {
		float data[ELEMENT_COUNT];
		float row_col[ROW_COUNT][ROW_COUNT];
	  };

	  Matrix3();

	  explicit Matrix3(float value);

	  Matrix3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);

	  Matrix3(const Matrix3& inMatrix);
	  Matrix3(Matrix3&& inMatrix) noexcept;
	  Matrix3& operator=(const Matrix3& inMatrix);
	  Matrix3& operator=(Matrix3&& inMatrix) noexcept;
	  Matrix3(const class Vector3& aVector, const class Vector3& bVector);

	  ~Matrix3() = default;

	  float* operator[](int i) const;
	  bool operator==(const Matrix3& rhs) const;
	  bool operator!=(const Matrix3& rhs) const;
	  Matrix3 operator+(const Matrix3& rhs) const;
	  Matrix3& operator+=(const Matrix3& rhs);
	  Matrix3 operator-(const Matrix3& rhs) const;
	  Matrix3 operator-=(const Matrix3& rhs);
	  Matrix3 operator*(const Matrix3& rhs) const;
	  Matrix3 operator*=(const Matrix3& rhs);
	  Matrix3 operator*(float scalar) const;
	  Matrix3 operator*=(float scalar);
	  class Vector3 operator*(const class Vector3& rhs) const;

	  float determinant() const;
	  float get(int x, int y) const;
	  void set(int x, int y, float number);
	  Matrix3 inverse() const;
	  Matrix3 transpose() const;
	  bool isIdentity() const;
	  bool isZero() const;
	  class Vector3 getRow(int row) const;
	  void setRow(int row, class Vector3 rowData);
	  class Vector3 getCol(int col) const;
	  void setCol(int col, class Vector3 colData);
	  void setDiagonal(const Vector3& diagonal);
	  void setDiagonal(float r0c0, float r1c1, float r2c2);
	  static bool fuzzyEqual(const Matrix3& lhs, const Matrix3& rhs);
  };

}

#endif
