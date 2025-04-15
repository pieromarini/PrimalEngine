#pragma once

#include <cstdint>
#include "data_structures/fixed_array.h"

namespace pm {

struct MemoryArena {
	uint64_t size;
	char* memory;
	char* allocated;
	char* committed;
};


// TODO(piero): This should be dynamic per arena
constexpr uint32_t PAGES_PER_COMMIT = 2;

#define KILOBYTE(value) ((value) * 1024)
#define MEGABYTE(value) (KILOBYTE(value) * 1024)
#define GIGABYTE(value) (MEGABYTE(value) * 1024)

#define MemoryArenaPush(T, count, arena) static_cast<T *>(MemoryArena_push(arena, sizeof(T) * count, alignof(T)))

#define MemoryArenaCreateArray(A, T, elementCount, arena) A{ .size = elementCount, .length = 0, .data = MemoryArenaPush(T, elementCount, arena) }

MemoryArena MemoryArena_create(uint64_t bytesToReserve);
void MemoryArena_destroy(MemoryArena* arena);
void MemoryArena_commit(MemoryArena* arena, uint64_t size);

void* MemoryArena_push(MemoryArena* arena, uint64_t size, uint64_t align);

void MemoryArena_pop(MemoryArena* arena, uint64_t size);

void MemoryArena_clear(MemoryArena* arena);

// Internal
void* MemoryArena_os_reserve(uint64_t bytesToReserve);
bool MemoryArena_os_commit(void* addr, uint64_t size);
bool MemoryArena_os_decommit(void* addr, uint64_t size);
void MemoryArena_os_release(void* addr, uint64_t size);
uint64_t MemoryArena_os_getPageSize();

char* alignMemory(char *ptr, uint32_t align);

}// namespace pm
