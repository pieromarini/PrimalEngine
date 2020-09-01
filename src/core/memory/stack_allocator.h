#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <iostream>
#include <cstdint>

#include "mem_util.h"

namespace primal {

  class StackAllocator {
	public:
	  using Marker = std::size_t;

	  StackAllocator() = delete;
	  explicit StackAllocator(std::size_t stackSize);
	  ~StackAllocator();

	  void* alloc(std::size_t size, uint8_t alignment);

	  template<typename T, typename... Args>
	  T* New(Args...);

	  void freeToMarker(const Marker marker) { top = marker; }

	  void clear() { top = 0; }

	  Marker getMarker() const { return top; }

	private:
	  Marker top;
	  std::size_t totalSize;
	  void* bottom;
	  std::uintptr_t bottomAddress;
  };

  template<typename T, typename... Args>
  T* StackAllocator::New(Args... args) {
	void* mem = alloc(sizeof(T), MemUtil::ALIGNMENT);
	return new (mem) T(args...);
  }

}

#endif
