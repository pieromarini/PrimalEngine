#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include <cassert>
#include <initializer_list>

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
	  };
	  matrix() {
		  for (std::size_t col = 0; col < n; ++col) {
			  for (std::size_t row = 0; row < m; ++row) {
				  e[col][row] = (col == row) ? T(1.0f) : T(0.0f);
			  }
		  }
	  }
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
	  vector<m, T>& operator[](const std::size_t colIndex) {
		  assert(colIndex >= 0 && colIndex < n);
		  return col[colIndex];
	  }

	  const vector<m, T>& operator[](const std::size_t colIndex) const {
		  assert(colIndex >= 0 && colIndex < n);
		  return col[colIndex];
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

}
#endif
