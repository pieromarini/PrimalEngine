#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include <array>
#include <cassert>
#include <initializer_list>

namespace primal::math {
/* 
      Generic vector template version supporting vectors of any type and any
      positive dimension. Constructors and templated vector mathematics are 
      defined outside the templated class to support shared functionality in
      combination with template specialization of vec2,vec3 and vec4.
      The underlying implementation of all the vector types are based on C++
      templates. The most common found vector classes are however defined with
      preprocessor types for better readability hiding the underlying template
      implementation. For vectors of a dimension > 4 it is required to directly
      specify the template variables.
    */
  template<std::size_t n, class T>
  struct vector {
	public:
	  std::array<T, n> data;

	  vector() {}
	  vector(const T& v) {
		  for (auto& el : data) {
			  el = v;
		  }
	  }
	  vector(const std::initializer_list<T> args) {
		  assert(args.size() < n);
		  data = args;
	  }

	  T& operator[](const std::size_t index) {
		  assert(index >= 0 && index < n);
		  return data.at(index);
	  }

	  T operator[](const std::size_t index) const {
		  assert(index >= 0 && index < n);
		  return data.at(index);
	  }

	  vector<n, T> operator-() const;
  };

  template<typename T>
  struct vector<2, T> {
	  union {
		  std::array<T, 2> data;
		  // position
		  struct
		  {
			  T x;
			  T y;
		  };
		  struct
		  {
			  T u;
			  T v;
		  };
		  struct
		  {
			  T s;
			  T t;
		  };
	  };

	  vector() {
		  data = {};
	  }
	  vector(const T& v) {
		  data = { v, v };
	  }
	  vector(const std::initializer_list<T> args) {
		  assert(args.size() <= 2);
		  int index = 0;
		  for (auto begin = args.begin(); begin != args.end(); ++begin) {
			  data.at(index++) = *begin;
		  }
	  }
	  vector(const T& x, const T& y) {
		  data = { x, y };
	  }

	  T& operator[](const std::size_t index) {
		  assert(index >= 0 && index < 2);
		  return data.at(index);
	  }

	  T operator[](const std::size_t index) const {
		  assert(index >= 0 && index < 2);
		  return data.at(index);
	  }

	  vector<2, T> operator-() const;
  };

  template<typename T>
  struct vector<3, T> {
	  union {
		  std::array<T, 3> data;
		  // position
		  struct
		  {
			  T x;
			  T y;
			  T z;
		  };
		  // color
		  struct
		  {
			  T r;
			  T g;
			  T b;
		  };
		  // texture coordinates
		  struct
		  {
			  T s;
			  T t;
			  T tt;
		  };
		  struct
		  {
			  vector<2, T> xy;
		  };
		  struct
		  {
			  vector<2, T> yz;
		  };
	  };

	  static vector<3, T> UP;
	  static vector<3, T> DOWN;
	  static vector<3, T> LEFT;
	  static vector<3, T> RIGHT;
	  static vector<3, T> FORWARD;
	  static vector<3, T> BACK;

	  vector() {
		  data = {};
	  }
	  vector(const T& v) {
		  data = { v, v, v };
	  }
	  vector(const std::initializer_list<T> args) {
		  assert(args.size() <= 3);
		  int index = 0;
		  for (auto begin = args.begin(); begin != args.end(); ++begin) {
			  data.at(index++) = *begin;
		  }
	  }
	  vector(const T& x, const T& y, const T& z) {
		  data = { x, y, z };
	  }
	  vector(const vector<2, T>& vec, const T& z) {
		  data = { vec.x, vec.y, z };
	  }
	  vector(const T& x, const vector<2, T>& vec) {
		  data = { x, vec.x, vec.y };
	  }

	  T& operator[](const std::size_t index) {
		  assert(index >= 0 && index < 3);
		  return data.at(index);
	  }

	  T operator[](const std::size_t index) const {
		  assert(index >= 0 && index < 3);
		  return data.at(index);
	  }

	  vector<3, T> operator-() const;
  };

  template<typename T>
  vector<3, T> vector<3, T>::UP = vector<3, T>(0.0, 1.0, 0.0);
  template<typename T>
  vector<3, T> vector<3, T>::DOWN = vector<3, T>(0.0, -1.0, 0.0);
  template<typename T>
  vector<3, T> vector<3, T>::LEFT = vector<3, T>(-1.0, 0.0, 0.0);
  template<typename T>
  vector<3, T> vector<3, T>::RIGHT = vector<3, T>(1.0, 0.0, 0.0);
  template<typename T>
  vector<3, T> vector<3, T>::FORWARD = vector<3, T>(0.0, 0.0, -1.0);
  template<typename T>
  vector<3, T> vector<3, T>::BACK = vector<3, T>(0.0, 0.0, 1.0);

  // 4D vector specialization
  template<typename T>
  struct vector<4, T> {
	  union {
		  std::array<T, 4> data;
		  // position
		  struct
		  {
			  T x;
			  T y;
			  T z;
			  T w;
		  };
		  // color
		  struct
		  {
			  T r;
			  T g;
			  T b;
			  T a;
		  };
		  // texture coordinates
		  struct
		  {
			  T s;
			  T t;
			  T tt;
		  };
		  struct
		  {
			  vector<2, T> xy;
			  T _ignored1;
			  T _ignored2;
		  };
		  struct
		  {
			  T _ignored11;
			  T _ignored21;
			  vector<2, T> yz;
		  };
		  struct
		  {
			  vector<3, T> xyz;
			  T _ignored12;
		  };
		  struct
		  {
			  vector<3, T> rgb;
			  T _ignored13;
		  };
		  struct
		  {
			  vector<3, T> srt;
			  T _ignored14;
		  };
	  };

	  vector() {
		  data = {};
	  }
	  vector(const T& v) {
		  data = { v, v, v, v };
	  }
	  vector(const std::initializer_list<T> args) {
		  assert(args.size() <= 4);
		  int index = 0;
		  for (auto begin = args.begin(); begin != args.end(); ++begin) {
			  data.at(index++) = *begin;
		  }
	  }
	  vector(const T& x, const T& y, const T& z, const T& w) {
		  data = { x, y, z, w };
	  }
	  vector(const vector<2, T>& xy, const vector<2, T>& zw) {
		  data = { xy.x, xy.y, zw.z, zw.w };
	  }
	  vector(const vector<3, T>& xyz, const T& w) {
		  data = { xyz.x, xyz.y, xyz.z, w };
	  }

	  T& operator[](const std::size_t index) {
		  assert(index >= 0 && index < 4);
		  return data.at(index);
	  }

	  T operator[](const std::size_t index) const {
		  assert(index >= 0 && index < 4);
		  return data.at(index);
	  }

	  vector<4, T> operator-() const;
  };

  using vec2 = vector<2, float>;
  using vec3 = vector<3, float>;
  using vec4 = vector<4, float>;
  using ivec2 = vector<2, int>;
  using ivec3 = vector<3, int>;
  using ivec4 = vector<4, int>;
  using dvec2 = vector<2, double>;
  using dvec3 = vector<3, double>;
  using dvec4 = vector<4, double>;

  template<std::size_t n, typename T>
  inline vector<n, T> vector<n, T>::operator-() const {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = -data[i];
	  }
	  return result;
  }
  template<typename T>
  inline vector<2, T> vector<2, T>::operator-() const {
	  return { -x, -y };
  }
  template<typename T>
  inline vector<3, T> vector<3, T>::operator-() const {
	  return { -x, -y, -z };
  }
  template<typename T>
  inline vector<4, T> vector<4, T>::operator-() const {
	  return { -x, -y, -z, -w };
  }

  template<std::size_t n, typename T>
  inline vector<n, T> operator+(const vector<n, T>& lhs, T scalar) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] + scalar;
	  }
	  return result;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator+(T scalar, const vector<n, T>& rhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i)
		  result[i] = rhs[i] + scalar;
	  return result;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator+(const vector<n, T>& lhs, const vector<n, T>& rhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i)
		  result[i] = lhs[i] + rhs[i];
	  return result;
  }

  template<std::size_t n, typename T>
  inline vector<n, T> operator-(const vector<n, T>& lhs, T scalar) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] - scalar;
	  }
	  return result;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator-(const vector<n, T>& lhs, const vector<n, T>& rhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] - rhs[i];
	  }
	  return result;
  }

  template<std::size_t n, typename T>
  inline vector<n, T> operator*(const vector<n, T>& lhs, T scalar) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] * scalar;
	  }
	  return result;
  }
  template<std::size_t n, typename T>
  vector<n, T>& operator*=(vector<n, T>& lhs, T scalar) {
	  for (std::size_t i = 0; i < n; ++i) {
		  lhs[i] *= scalar;
	  }
	  return lhs;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator*(T scalar, const vector<n, T>& lhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] * scalar;
	  }
	  return result;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator*(const vector<n, T>& lhs, const vector<n, T>& rhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] * rhs[i];
	  }
	  return result;
  }

  template<std::size_t n, typename T>
  inline vector<n, T> operator/(const vector<n, T>& lhs, T scalar) {
	  vector<n, T> result;
	  for (unsigned int i = 0; i < n; ++i) {
		  result[i] = lhs[i] / scalar;
	  }
	  return result;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator/(T scalar, const vector<n, T>& rhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = rhs[i] / scalar;
	  }
	  return result;
  }
  template<std::size_t n, typename T>
  inline vector<n, T> operator/(const vector<n, T>& lhs, const vector<n, T>& rhs) {
	  vector<n, T> result;
	  for (std::size_t i = 0; i < n; ++i) {
		  result[i] = lhs[i] / rhs[i];
	  }
	  return result;
  }

}

#endif
