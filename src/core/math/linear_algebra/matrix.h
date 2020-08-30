#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include <cassert>
#include <initializer_list>
#include <stdexcept>

#include "vector.h"

namespace primal::math {
  /* 
	Generic m by n dimensional matrix template type version supporting matrices of any type. The 
	matrix type follows in functionality conventions from the mathematical literature. By default 
	we will only be using floating point matrices, but having a generic version allows us to 
	potentially use double precision matrices as well (or even integer matrices).
	The matrices are stored in column-major order, the resulting transformations will also assume 
	column-major order, keeping matrix-vector multiplications with the matrix on the left side of 
	the equation and representing vectors as column vectors (post-multiplication).
	Matrix numbering by math conventions:
	|  0  1  2  3 |
	|  4  5  6  7 |
	|  8  9 10 11 |
	| 12 13 14 15 |
	Column-major layout in memory:
	[ 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 ]
	Matrix numbering if we access column-major memory sequentially in the array:
	|  0  4  8 12 |
	|  1  5  9 13 |
	|  2  6 10 14 |
	|  3  7 11 15 |
	There is no need for matrix template specialization.
  */
  template<std::size_t m, std::size_t n, typename T>
  struct matrix {
	  union {
		  T e[n][m];
		  struct {
			  vector<m, T> col[n];
		  };
		  T data[n * m];
	  };

	  // Identity Matrix
	  matrix() = default;

	  /*
	  matrix() {
		  for (std::size_t col = 0; col < n; ++col) {
			  for (std::size_t row = 0; row < m; ++row) {
				  e[col][row] = (col == row) ? T(1.0f) : T(0.0f);
			  }
		  }
	  }
	  */

	  matrix(const std::initializer_list<T> args) {
		assert(args.size() <= m * n);
		std::size_t cols = 0, rows = 0;

		for (auto& it : args) {
		  e[cols][rows++] = it;
		  if (rows >= m) {
			++cols;
			rows = 0;
		  }
		}
	  }

	  // Returns column vector
	  vector<m, T>& operator[](const std::size_t colIndex) {
		assert(colIndex >= 0 && colIndex < n);
		return col[colIndex];
	  }

	  const vector<m, T>& operator[](const std::size_t colIndex) const {
		assert(colIndex >= 0 && colIndex < n);
		return col[colIndex];
	  }

	  // NOTE: For now, only set diagonal if matrix is 4x4
	  void setDiagonal(const vec4& diagonal) {
		assert(n == 4 && m == 4);

		for (auto i = 0; i < n; ++i) {
		  for (auto j = 0; j < m; ++j) {
			if (i == j)
			  e[i][j] = diagonal[i];
		  }
		}
	  }

	  void setCol(const std::size_t colIndex, const vec4& colData) {
		assert(n == 4 && m == 4);

		for (auto i = 0; i < m; ++i) {
		  e[colIndex][i] = colData[i]; 
		}
	  }

	  void setTopLeftMatrix3(const matrix<3, 3, float>& matrix) {
		for (auto i = 0; i < 3; ++i)
		  setCol(i, { matrix[i], e[i][3] });
	  }
  };

  using mat2 = matrix<2, 2, float>;
  using mat3 = matrix<3, 3, float>;
  using mat4 = matrix<4, 4, float>;
  using dmat2 = matrix<2, 2, double>;
  using dmat3 = matrix<3, 3, double>;
  using dmat4 = matrix<4, 4, double>;

  template<std::size_t m, std::size_t n, typename T>
  matrix<m, n, T> operator+(const matrix<m, n, T>& lhs, const matrix<m, n, T>& rhs) {
	  matrix<m, n, T> result;
	  for (std::size_t col = 0; col < n; ++col) {
		  for (std::size_t row = 0; row < m; ++row) {
			  result[col][row] = lhs[col][row] + rhs[col][row];
		  }
	  }
	  return result;
  }
  template<std::size_t m, std::size_t n, typename T>
  matrix<m, n, T> operator-(const matrix<m, n, T>& lhs, const matrix<m, n, T>& rhs) {
	  matrix<m, n, T> result;
	  for (std::size_t col = 0; col < n; ++col) {
		  for (std::size_t row = 0; row < m; ++row) {
			  result[col][row] = lhs[col][row] - rhs[col][row];
		  }
	  }
	  return result;
  }
  template<std::size_t m, std::size_t n, std::size_t o, typename T>
  matrix<m, o, T> operator*(const matrix<m, n, T>& lhs, const matrix<n, o, T>& rhs) {
	  matrix<m, o, T> result;
	  for (std::size_t col = 0; col < o; ++col) {
		  for (std::size_t row = 0; row < m; ++row) {
			  T value = {};
			  for (std::size_t j = 0; j < n; ++j) {
				  value += lhs[j][row] * rhs[col][j];
			  }
			  result[col][row] = value;
		  }
	  }
	  return result;
  }
  template<std::size_t m, std::size_t n, std::size_t o, typename T>
  matrix<m, o, T>& mul(matrix<m, n, T>& result, const matrix<m, n, T>& lhs, const matrix<n, o, T>& rhs) {
	  for (std::size_t col = 0; col < o; ++col) {
		  for (std::size_t row = 0; row < m; ++row) {
			  T value = {};
			  for (std::size_t j = 0; j < n; ++j) {
				  value += lhs[j][row] * rhs[col][j];
			  }
			  result[col][row] = value;
		  }
	  }
	  return result;
  }

  template<std::size_t m, std::size_t n, typename T>
  vector<m, T> operator*(const matrix<m, n, T>& lhs, const vector<n, T>& rhs) {
	vector<m, T> result;
	for (std::size_t row = 0; row < m; ++row) {
	  T value = {};
	  for (std::size_t j = 0; j < n; ++j) {
		value += lhs[j][row] * rhs[j];
	  }
	  result[row] = value;
	}
	return result;
  }

  template<std::size_t m, std::size_t n, typename T>
  matrix<m, n, T> operator*(const matrix<m, n, T>& lhs, float rhs) {
	matrix<m, n, T> result;
	for (std::size_t row = 0; row < m; ++row) {
	  for (std::size_t column = 0; column < n; ++column) {
		result[row][column] = lhs[row][column] * rhs;
	  }
	}
	return result;
  }

  inline mat4 inverse(const mat4& mat) {
	float m11 = mat.data[5] * mat.data[10] * mat.data[15] - mat.data[5] * mat.data[11] * mat.data[14] -
				mat.data[9] * mat.data[6] * mat.data[15] + mat.data[9] * mat.data[7] * mat.data[14] +
				mat.data[13] * mat.data[6] * mat.data[11] - mat.data[13] * mat.data[7] * mat.data[10];
	float m21 = -mat.data[4] * mat.data[10] * mat.data[15] + mat.data[4] * mat.data[11] * mat.data[14] +
				mat.data[8] * mat.data[6] * mat.data[15] - mat.data[8] * mat.data[7] * mat.data[14] -
				mat.data[12] * mat.data[6] * mat.data[11] + mat.data[12] * mat.data[7] * mat.data[10];
	float m31 = mat.data[4] * mat.data[9] * mat.data[15] - mat.data[4] * mat.data[11] * mat.data[13] -
				mat.data[8] * mat.data[5] * mat.data[15] + mat.data[8] * mat.data[7] * mat.data[13] +
				mat.data[12] * mat.data[5] * mat.data[11] - mat.data[12] * mat.data[7] * mat.data[9];
	float m41 = -mat.data[4] * mat.data[9] * mat.data[14] + mat.data[4] * mat.data[10] * mat.data[13] +
				mat.data[8] * mat.data[5] * mat.data[14] - mat.data[8] * mat.data[6] * mat.data[13] -
				mat.data[12] * mat.data[5] * mat.data[10] + mat.data[12] * mat.data[6] * mat.data[9];
	float m12 = -mat.data[1] * mat.data[10] * mat.data[15] + mat.data[1] * mat.data[11] * mat.data[14] +
				mat.data[9] * mat.data[2] * mat.data[15] - mat.data[9] * mat.data[3] * mat.data[14] -
				mat.data[13] * mat.data[2] * mat.data[11] + mat.data[13] * mat.data[3] * mat.data[10];
	float m22 = mat.data[0] * mat.data[10] * mat.data[15] - mat.data[0] * mat.data[11] * mat.data[14] -
				mat.data[8] * mat.data[2] * mat.data[15] + mat.data[8] * mat.data[3] * mat.data[14] +
				mat.data[12] * mat.data[2] * mat.data[11] - mat.data[12] * mat.data[3] * mat.data[10];
	float m32 = -mat.data[0] * mat.data[9] * mat.data[15] + mat.data[0] * mat.data[11] * mat.data[13] +
				mat.data[8] * mat.data[1] * mat.data[15] - mat.data[8] * mat.data[3] * mat.data[13] -
				mat.data[12] * mat.data[1] * mat.data[11] + mat.data[12] * mat.data[3] * mat.data[9];
	float m42 = mat.data[0] * mat.data[9] * mat.data[14] - mat.data[0] * mat.data[10] * mat.data[13] -
				mat.data[8] * mat.data[1] * mat.data[14] + mat.data[8] * mat.data[2] * mat.data[13] +
				mat.data[12] * mat.data[1] * mat.data[10] - mat.data[12] * mat.data[2] * mat.data[9];
	float m13 = mat.data[1] * mat.data[6] * mat.data[15] - mat.data[1] * mat.data[7] * mat.data[14] -
				mat.data[5] * mat.data[2] * mat.data[15] + mat.data[5] * mat.data[3] * mat.data[14] +
				mat.data[13] * mat.data[2] * mat.data[7] - mat.data[13] * mat.data[3] * mat.data[6];
	float m23 = -mat.data[0] * mat.data[6] * mat.data[15] + mat.data[0] * mat.data[7] * mat.data[14] +
				mat.data[4] * mat.data[2] * mat.data[15] - mat.data[4] * mat.data[3] * mat.data[14] -
				mat.data[12] * mat.data[2] * mat.data[7] + mat.data[12] * mat.data[3] * mat.data[6];
	float m33 = mat.data[0] * mat.data[5] * mat.data[15] - mat.data[0] * mat.data[7] * mat.data[13] -
				mat.data[4] * mat.data[1] * mat.data[15] + mat.data[4] * mat.data[3] * mat.data[13] +
				mat.data[12] * mat.data[1] * mat.data[7] - mat.data[12] * mat.data[3] * mat.data[5];
	float m43 = -mat.data[0] * mat.data[5] * mat.data[14] + mat.data[0] * mat.data[6] * mat.data[13] +
				mat.data[4] * mat.data[1] * mat.data[14] - mat.data[4] * mat.data[2] * mat.data[13] -
				mat.data[12] * mat.data[1] * mat.data[6] + mat.data[12] * mat.data[2] * mat.data[5];
	float m14 = -mat.data[1] * mat.data[6] * mat.data[11] + mat.data[1] * mat.data[7] * mat.data[10] +
				mat.data[5] * mat.data[2] * mat.data[11] - mat.data[5] * mat.data[3] * mat.data[10] -
				mat.data[9] * mat.data[2] * mat.data[7] + mat.data[9] * mat.data[3] * mat.data[6];
	float m24 = mat.data[0] * mat.data[6] * mat.data[11] - mat.data[0] * mat.data[7] * mat.data[10] -
				mat.data[4] * mat.data[2] * mat.data[11] + mat.data[4] * mat.data[3] * mat.data[10] +
				mat.data[8] * mat.data[2] * mat.data[7] - mat.data[8] * mat.data[3] * mat.data[6];
	float m34 = -mat.data[0] * mat.data[5] * mat.data[11] + mat.data[0] * mat.data[7] * mat.data[9] +
				mat.data[4] * mat.data[1] * mat.data[11] - mat.data[4] * mat.data[3] * mat.data[9] -
				mat.data[8] * mat.data[1] * mat.data[7] + mat.data[8] * mat.data[3] * mat.data[5];
	float m44 = mat.data[0] * mat.data[5] * mat.data[10] - mat.data[0] * mat.data[6] * mat.data[9] -
				mat.data[4] * mat.data[1] * mat.data[10] + mat.data[4] * mat.data[2] * mat.data[9] +
				mat.data[8] * mat.data[1] * mat.data[6] - mat.data[8] * mat.data[2] * mat.data[5];

	float det = mat.data[0] * m11 + mat.data[1] * m21 + mat.data[2] * m31 + mat.data[3] * m41;

	if (det == 0) {
	  throw std::out_of_range{"Matrix4::Inverse => Cannot do inverse when the determinant is zero."};
	}

	mat4 ret{ m11, m12, m13, m14, m21, m22, m23, m24,
			  m31, m32, m33, m34, m41, m42, m43, m44 };
	return ret * (1.f / det);
  }

}
#endif
