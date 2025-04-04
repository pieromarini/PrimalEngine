#pragma once

#include <cstdint>

namespace pm {

struct MemoryArena {
	uint64_t size;
	uint64_t pos;
	char* memory;
};

constexpr uint32_t PAGES_PER_COMMIT = 2;

#define KILOBYTE(value) ((value) * 1024)
#define MEGABYTE(value) (KILOBYTE(value) * 1024)
#define GIGABYTE(value) (MEGABYTE(value) * 1024)

#define MemoryArenaPush(T, count, arena) (T *) MemoryArena_push(arena, size_of(T) * count)

MemoryArena MemoryArena_alloc(uint64_t bytesToReserve);
void MemoryArena_free(MemoryArena* arena);

void* MemoryArena_push(MemoryArena* arena, uint64_t size);

void MemoryArena_pop(MemoryArena* arena, uint64_t size);
uint64_t MemoryArena_pos(MemoryArena* arena);

void MemoryArena_clear(MemoryArena* arena);

// Internal
void* MemoryArena_os_reserve(uint64_t bytesToReserve);
bool MemoryArena_os_commit(void* addr, uint64_t size);
bool MemoryArena_os_decommit(void* addr, uint64_t size);
void MemoryArena_os_release(void* addr, uint64_t size);
uint64_t MemoryArena_os_getPageSize();

}// namespace pm
