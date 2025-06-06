#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>
#include <iostream>

namespace pm {

template<typename T>
struct FixedArray {
	uint32_t size{};
	uint32_t length{};
	T* data{};
};

template<typename T>
inline T* FixedArray_get(FixedArray<T>& array, uint32_t index) {
	assert(index < array.length); // TODO(piero): handle error
	return &array.data[index];
}

template<typename T>
inline T FixedArray_getValue(FixedArray<T>& array, uint32_t index) {
	assert(index < array.length); // TODO(piero): handle error
	return array.data[index];
}

template<typename T>
inline T* FixedArray_back(FixedArray<T>& array) {
	assert((array.length - 1) >= 0);
	return &array.data[array.length - 1];
}

template<typename T>
inline T* FixedArray_add(FixedArray<T>& array, T value) {
	assert(array.length < array.size); // TODO(piero): handle error
	array.data[array.length++] = value;

	return &array.data[array.length - 1];
}

template<typename T>
inline void FixedArray_set(FixedArray<T>& array, uint32_t index, T value) {
	assert(index < array.size); // TODO(piero): handle error
	array.data[index] = value;
	array.length += 1;
}

// NOTE(piero): this is remove + swapback
// TODO(piero): test this
template<typename T>
inline T FixedArray_remove(FixedArray<T>& array, uint32_t index) {
	assert(index < array.length); // TODO(piero): handle error

	array.length--;
	auto removed = array.data[index];
	array.data[index] = array.data[array.length];

	return removed;
}

template<typename T>
inline void FixedArray_removeRange(FixedArray<T>& array, uint32_t startIndex, uint32_t endIndex) {
	assert(endIndex <= array.length); // TODO(piero): handle error

	array.length -= (endIndex - startIndex);
	for (uint32_t i = startIndex; i < endIndex; ++i) {
		array.data[i] = {};
	}
}

template<typename T>
inline void FixedArray_print(FixedArray<T>& array) {
	std::cout << "FixedArray: ";
	for (int32_t i = 0; i < array.length; ++i) {
		auto item = FixedArray_getValue(array, i);
		std::cout << item << ' ';
	}
	std::cout << '\n';
}

// NOTE(piero): this breaks for non-comparable type T's
template<typename T>
inline int32_t FixedArray_findIndex(FixedArray<T>& array, T value) {
	int32_t found = -1;

	for (int32_t i = 0; i < array.length; ++i) {
		auto item = FixedArray_get(array, i);
		if (*item == value) {
			found = i;
			break;
		}
	}

	return found;
}

template<typename T>
inline int32_t FixedArray_copy(FixedArray<T>& array, T value) {
	int32_t found = -1;

	for (int32_t i = 0; i < array.length; ++i) {
		auto item = FixedArray_get(array, i);
		if (*item == value) {
			found = i;
			break;
		}
	}

	return found;
}

template<typename T>
inline void FixedArray_clear(FixedArray<T>& array) {
	// Clear to zero
	memset(array.data, 0, array.length * sizeof(T));
	array.length = 0;
}

template<typename T>
inline bool FixedArray_empty(FixedArray<T>& array) {
	return array.length == 0;
}

// Stack-like interface
template<typename T>
inline T* FixedArray_top(FixedArray<T>& array) {
	return FixedArray_back(array);
}

template<typename T>
inline void FixedArray_pop(FixedArray<T>& array) {
	assert(array.length > 0);
	array.length--;
}


}
