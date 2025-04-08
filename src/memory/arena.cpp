#include "arena.h"
#include <cassert>
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

// Each arena will do a virtual alloc.
MemoryArena MemoryArena_alloc(uint64_t bytesToReserve) {
	MemoryArena arena{};

	arena.memory = static_cast<char*>(MemoryArena_os_reserve(bytesToReserve));
	arena.size = bytesToReserve;
	arena.pos = 0;

	// Don't allow application to continue if we can't allocate.
	// assert(arena.memory != MAP_FAILED);

	std::cout << std::format("Allocated {} bytes\n", bytesToReserve);

	return arena;
}

void MemoryArena_free(MemoryArena* arena) {
#ifdef PLATFORM_WINDOWS
#endif

#ifdef PLATFORM_POSIX
	munmap(arena->memory, arena->size);
	arena->memory = nullptr;
	arena->size = 0;
	arena->pos = 0;
#endif
}

// TODO: implement alignment
void* MemoryArena_push(MemoryArena* arena, uint64_t size) {
	assert(arena->pos + size <= arena->size);

	arena->pos += size;

	return arena->memory;
}

void MemoryArena_pop(MemoryArena* arena, uint64_t size) {
	assert(arena->pos >= size);

	for (uint64_t i = arena->pos; i > arena->pos - size; --i) {
		arena->memory[i] = 0;
	}
	arena->pos -= size;
}

uint64_t MemoryArena_pos(MemoryArena* arena) {
	return arena->pos;
}

void MemoryArena_clear(MemoryArena* arena) {
	// This should just 0 out all of the arena's memory.
	for (uint64_t i = 0; i < arena->size; ++i) {
		arena->memory[i] = 0;
	}
}

void* MemoryArena_os_reserve(uint64_t bytesToReserve) {
	void* mem = nullptr;

#ifdef PLATFORM_WINDOWS
	mem = VirtualAlloc(nullptr, bytesToReserve, MEM_RESERVE, PAGE_NOACCESS);
#endif

#ifdef PLATFORM_POSIX
	mem = mmap(nullptr, bytesToReserve, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

	return mem;
}

bool MemoryArena_os_commit(void* addr, uint64_t size) {
	void* mem{};

#ifdef PLATFORM_WINDOWS
	auto ptr = VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
	assert(ptr);
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
