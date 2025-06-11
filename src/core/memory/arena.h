#pragma once

#include "core/core.h"

namespace pm {

constexpr u32 ARENA_HEADER_SIZE = 128;

using ArenaFlags = u32;
enum {
	ArenaFlag_NoChain = (1 << 0),
};


static constexpr u64 arenaDefaultReserveSize = Megabytes(64);
static constexpr u64 arenaDefaultCommitSize = Kilobytes(64);
static constexpr ArenaFlags arenaDefaultFlags = 0;

struct ArenaParams {
	ArenaFlags flags{ arenaDefaultFlags };
	u64 reserveSize{ arenaDefaultReserveSize };
	u64 commitSize{ arenaDefaultCommitSize };
	void* optionalBackingBuffer{};
	String8 name{};
};

struct Arena {
	Arena* prev;
	Arena* current;
	ArenaFlags flags;
	u32 nameSize;
	u64 commitedSize;
	u64 reservedSize;
	u64 basePos;
	u64 pos;
	u64 commited;
	u64 reserved;
};

struct Temp {
	Arena* arena;
	u64 pos;
};

Arena* arenaAlloc(u64 size);
Arena* arenaAlloc(ArenaParams params);
#define ArenaAllocDefault() arenaAlloc((ArenaParams){ .flags = arenaDefaultFlags, .reserveSize = arenaDefaultReserveSize, .commitSize = arenaDefaultCommitSize })
void arenaRelease(Arena* arena);

String8 arenaName(Arena* arena);

void* arenaPush(Arena* arena, u64 size, u64 align);
u64 arenaPos(Arena* arena);
void arenaPopTo(Arena* arena, u64 pos);

void arenaClear(Arena* arena);
void arenaPop(Arena* arena, u64 amt);

Temp tempBegin(Arena* arena);
void tempEnd(Temp temp);

#define PushArrayNoZeroAligned(a, T, c, align) (T*)arenaPush((a), sizeof(T) * (c), (align))
#define PushArrayAligned(a, T, c, align) (T*)MemoryZero(PushArrayNoZeroAligned(a, T, c, align), sizeof(T) * (c))

#define PushArrayNoZero(a, T, c) PushArrayNoZeroAligned(a, T, c, Max(8, alignof(T)))
#define PushStructNoZero(a, T) PushArrayNoZeroAligned(a, T, 1, Max(8, alignof(T)))

#define PushArray(a, T, c) PushArrayAligned(a, T, c, Max(8, alignof(T)))
#define PushStruct(a, T) PushArrayAligned(a, T, 1, Max(8, alignof(T)))

}// namespace pm
