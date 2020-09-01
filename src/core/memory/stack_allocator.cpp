#include "stack_allocator.h"
#include "tools/log.h"

namespace primal {

  StackAllocator::StackAllocator(std::size_t stackSize) : top{ 0 }, totalSize{ stackSize } {
	bottom = std::malloc(stackSize);
	bottomAddress = reinterpret_cast<std::uintptr_t>(bottom);
  }

  StackAllocator::~StackAllocator() {
	std::free(bottom);
  }

  void* StackAllocator::alloc(std::size_t size, uint8_t alignment) {
	MemUtil::checkAlignment(alignment);

	std::uintptr_t rawAddress = bottomAddress + top;
	std::uintptr_t misAlignment = rawAddress & (alignment - 1);
	std::ptrdiff_t adjustment = alignment - misAlignment;

	adjustment = adjustment & (alignment - 1);
	std::uintptr_t alignedAddress = rawAddress + adjustment;
	Marker newTop = top + size + adjustment;

	if (newTop > totalSize) {
	  PRIMAL_CORE_ERROR("StackAllocator::alloc: Not enough memory.");
	}

	top = newTop;

	return reinterpret_cast<void*>(alignedAddress);
  }

}
