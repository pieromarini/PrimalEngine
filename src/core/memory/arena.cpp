#include "arena.h"
#include <cassert>
#include <cstring>
#include <format>
#include <iostream>

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__APPLE__)
#define PLATFORM_POSIX
#endif

#if defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#elif defined(PLATFORM_POSIX)
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace pm {

char* alignMemory(char* ptr, uint32_t align) {
	auto result = reinterpret_cast<uint64_t>(ptr);
	uint64_t remainder = result % align;
	if (remainder != 0) {
		result += align - remainder;
	}

	return reinterpret_cast<char*>(result);
}

MemoryArena MemoryArena_create(uint64_t bytesToReserve) {
	MemoryArena arena{};

	arena.memory = static_cast<char*>(MemoryArena_os_reserve(bytesToReserve));
	arena.allocated = arena.memory;
	arena.committed = arena.memory;
	arena.size = bytesToReserve;

	// std::cout << std::format("Allocated {} bytes\n", bytesToReserve);

	return arena;
}

void MemoryArena_destroy(MemoryArena* arena) {
	MemoryArena_os_release(arena->memory, 0);
	arena->memory = nullptr;
	arena->allocated = nullptr;
	arena->size = 0;
}

void MemoryArena_commit(MemoryArena* arena, uint64_t size) {
	MemoryArena_os_commit(arena->allocated, size);
	arena->committed += size;
}

void* MemoryArena_push(MemoryArena* arena, uint64_t size, uint64_t align) {
	auto ptr = alignMemory(arena->allocated, align);
	arena->allocated = ptr + size;

	if (arena->committed < arena->allocated) {
		uint64_t granularity = MemoryArena_os_getPageSize() * PAGES_PER_COMMIT;
		auto sizeToCommit = (uint64_t)(arena->allocated - arena->committed);
		sizeToCommit += -sizeToCommit & (granularity - 1);

		auto result = MemoryArena_os_commit(arena->committed, sizeToCommit);
		assert(result);

		arena->committed += sizeToCommit;
	}

	return ptr;
}

// TODO(piero): I don't think this is working correctly?
void MemoryArena_pop(MemoryArena* arena, uint64_t size) {
	arena->allocated -= size;

	auto startIndex = static_cast<uint64_t>(arena->allocated - arena->memory) - 1;
	uint64_t endIndex = startIndex + size;
	for (uint64_t i = startIndex; i < endIndex; i++) {
		arena->allocated[i] = 0;
	}
}

// TODO(piero): Resets pointer to start of arena memory. 
// 							Doesn't clear memory to 0. Could provide this as an option or another function.
void MemoryArena_clear(MemoryArena* arena) {
	/* TODO(piero): decommit on clear?
	uint64_t commit_size = arena->committed - arena->memory;
	uint64_t page_size = MemoryArena_os_getPageSize();

	// If committed pages > 16, decommit pages after 16th
	uint64_t page_limit = page_size * 16;
	if (commit_size > page_limit) {
		char* start_addr = arena->memory + page_limit;
		MemoryArena_os_decommit(start_addr, commit_size - page_limit);
		arena->committed = start_addr;
	}
	*/

	arena->allocated = arena->memory;
}

void* MemoryArena_os_reserve(uint64_t bytesToReserve) {
	void* mem = nullptr;

#ifdef PLATFORM_WINDOWS
	mem = VirtualAlloc(nullptr, bytesToReserve, MEM_RESERVE, PAGE_NOACCESS);
#endif

#ifdef PLATFORM_POSIX
	mem = mmap(nullptr, bytesToReserve, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
	assert(mem);

	return mem;
}

bool MemoryArena_os_commit(void* addr, uint64_t size) {
	void* mem{};

#ifdef PLATFORM_WINDOWS
	auto ptr = VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
	assert(ptr); // We probably ran out of memory in our arena
#endif

#ifdef PLATFORM_POSIX
	uint32_t err = mprotect(addr, size, PROT_WRITE | PROT_READ);
#endif

	return true;
}

bool MemoryArena_os_decommit(void* addr, uint64_t size) {
#ifdef PLATFORM_WINDOWS
	auto ptr = VirtualFree(addr, size, MEM_DECOMMIT);
	assert(ptr);
#endif

#ifdef PLATFORM_POSIX
	uint32_t err = mprotect(addr, size, PROT_NONE);
	assert(err == 0);
#endif

	return true;
}

void MemoryArena_os_release(void* addr, uint64_t size) {
#ifdef PLATFORM_WINDOWS
	VirtualFree(addr, size, MEM_RELEASE);
#endif

#ifdef PLATFORM_POSIX
	munmap(addr, size);
#endif
}

uint64_t MemoryArena_os_getPageSize() {
	uint64_t result = 0;

#ifdef PLATFORM_WINDOWS
	SYSTEM_INFO info = { 0 };
	GetSystemInfo(&info);
	result = info.dwPageSize;
#endif

#ifdef PLATFORM_POSIX
	result = getpagesize();
#endif

	return result;
}

}// namespace pm
