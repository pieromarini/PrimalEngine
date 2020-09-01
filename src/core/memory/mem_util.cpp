#include "mem_util.h"
#include "core/core.h"
#include "tools/log.h"
#include <cstdint>

namespace primal {

  void MemUtil::checkAlignment(uint8_t alignment) {

	const bool isValid = alignment >= 8 && alignment <= 128 && (alignment & (alignment - 1)) == 0;
	if (!isValid) {
	  PRIMAL_CORE_ERROR("MemoryAllocator::checkAlignment: Illegal alignment.");
	}
  }

  void* MemUtil::alloc(std::size_t size, uint8_t alignment) {
	checkAlignment(alignment);

	const std::size_t expandedSize = size + alignment;
	const std::uintptr_t rawAddress = reinterpret_cast<std::uintptr_t>(std::malloc(expandedSize));
	const std::uintptr_t misAlignment = rawAddress & (alignment - 1);
	const uint8_t adjustment = alignment - static_cast<uint8_t>(misAlignment);
	const std::uintptr_t alignedAddress = rawAddress + adjustment;
	uint8_t* alignedMemory = reinterpret_cast<uint8_t*>(alignedAddress);
	alignedMemory[-1] = adjustment;

	PRIMAL_CORE_ASSERT("Incorrectly aligned memory.", reinterpret_cast<std::uintptr_t>(alignedMemory) % alignment == 0);

	return static_cast<void*>(alignedMemory);
  }

  void MemUtil::free(void* memoryPtr) {
	const uint8_t* alignedMemory = reinterpret_cast<uint8_t*>(memoryPtr);
	const std::ptrdiff_t adjustment = static_cast<std::ptrdiff_t>(alignedMemory[-1]);

	const std::uintptr_t alignedAddress = reinterpret_cast<std::uintptr_t>(memoryPtr);
	const std::uintptr_t rawAddress = alignedAddress - adjustment;
	void* rawMem = reinterpret_cast<void*>(rawAddress);
	std::free(rawMem);
  }

}
